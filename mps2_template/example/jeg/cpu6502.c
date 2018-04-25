#include "cpu6502.h"
#include "cpu6502_opcodes.h"
#include "cpu6502_debug.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "jeg_cfg.h"


#define SET_FLAGS(v)        do {cpu->chStatus = (v);} while(0)
#define GET_FLAGS()         (cpu->chStatus)

#define __BV(__N)           ((uint32_t)1<<(__N))

#define RECALC_ZN(v)                                                            \
            do{                                                                 \
                cpu->status_Z=(v)?0:1;                                          \
                cpu->status_N=(v)&0x80?1:0;                                     \
            } while(0)

#define PUSH(v)                                                                 \
    do {                                                                        \
        cpu->write(cpu->reference, 0x100|cpu->reg_SP, (v)&0xFF);                \
        cpu->reg_SP--;                                                          \
        if (cpu->reg_SP<0) {                                                    \
            cpu->reg_SP=0xFF;                                                   \
        }                                                                       \
    } while(0)

#if JEG_USE_EXTRA_16BIT_BUS_ACCESS == ENABLED
#define READ16(adr)     (cpu->readw(cpu->reference, adr))
#else
#define READ16(adr)     (cpu->read(cpu->reference, adr)|(cpu->read(cpu->reference, (adr)+1)<<8))
#endif
#define READ16BUG(adr)  (   (cpu->read(     cpu->reference, adr))               \
                        |   ((cpu->read(    cpu->reference,     ((adr)&0xFF00)  \
                                                            +   (((adr)+1)&0xFF)\
                                       )<<8)))

#define PAGE_DIFFERS(a,b) (((a)&0xFF00)!=((b)&0xFF00))

#define BRANCH(c)                                                               \
    do {                                                                        \
        if (c) {                                                                \
            cycles_passed+=PAGE_DIFFERS(cpu->reg_PC, address)?2:1;              \
            cpu->reg_PC=address;                                                \
        }                                                                       \
    } while(0)

#define COMPARE(a, b)                                                           \
    do {                                                                        \
        RECALC_ZN((a)-(b));                                                     \
        cpu->status_C= (a)>=(b)?1:0;                                            \
    } while(0)


#if  JEG_USE_DUMMY_READS == ENABLED
#   define DUMMY_READ(adr)
#else
#   define DUMMY_READ(adr) cpu->read(cpu->reference, adr);
#endif

#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED ||                          \
    JEG_USE_EXTRA_16BIT_BUS_ACCESS       == ENABLED
bool cpu6502_init(cpu6502_t *ptCPU, cpu6502_cfg_t *ptCFG) 
{
    do {
        if (NULL == ptCPU || NULL == ptCFG) {
            break;
        } else if (     (NULL == ptCFG->reference)
                #if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED
                    ||  (NULL == ptCFG->fnDMAGetSourceAddress)
                #endif
                #if JEG_USE_EXTRA_16BIT_BUS_ACCESS == ENABLED
                    ||  (NULL == ptCFG->readw)
                    ||  (NULL == ptCFG->writew)
                #endif
                    ||  (NULL == ptCFG->read)
                    ||  (NULL == ptCFG->write)) {
            break;
        }
    
        ptCPU->reference =              ptCFG->reference;
        ptCPU->read =                   ptCFG->read;
        ptCPU->write =                  ptCFG->write;
    #if JEG_USE_EXTRA_16BIT_BUS_ACCESS == ENABLED
        ptCPU->readw =                  ptCFG->readw;
        ptCPU->writew =                 ptCFG->writew;
    #endif
    #if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED
        ptCPU->fnDMAGetSourceAddress =  ptCFG->fnDMAGetSourceAddress;
    #endif
        return true;
    } while(false);

    return false;
}
#else
void cpu6502_init(cpu6502_t *cpu, void *reference, cpu6502_read_func_t read, cpu6502_write_func_t write) 
{
    cpu->reference = reference;
    cpu->read=read;
    cpu->write=write;
}
#endif

void cpu6502_reset(cpu6502_t *cpu) {
  // load program counter with address stored at 0xFFFC (low byte) and 0xFFFD (high byte)
    cpu->reg_A = 0;
    cpu->reg_X = 0;
    cpu->reg_Y = 0;
    
    cpu->reg_PC = READ16(0xFFFC);
    cpu->reg_SP = 0xFD; // reset stack pointer
    SET_FLAGS(0x24); // set following status flags: UNUSED, INTERRUPT
    cpu->cycle_number = 0;
    cpu->interrupt_pending = INTERRUPT_NONE;
    cpu->stall_cycles = 0;
}


int cpu6502_run(cpu6502_t *cpu, int cycles_to_run) {
  const opcode_tbl_entry_t *ptOpcode;
  int cycles_passed; // cycles used in one iteration
  int address=0; // calculated address for memory interaction
  int temp_value, temp_value2; // temporary value used for calculation

  do {
    cycles_passed=0;

    // check if cpu is stalling
    if (cpu->stall_cycles) {
      cycles_passed=cpu->stall_cycles;
      cpu->stall_cycles=0;
    }

    // check for interrupts
    if (cpu->interrupt_pending!=INTERRUPT_NONE) {
      PUSH(cpu->reg_PC>>8); // high byte of program counter to stack
      PUSH(cpu->reg_PC); // low byte
      PUSH(GET_FLAGS() | 0x10); // push status flags to stack
      if (cpu->interrupt_pending==INTERRUPT_NMI) {
        cpu->reg_PC=READ16(0xFFFA);
      }
      else {
        cpu->reg_PC=READ16(0xFFFE);
      }
      cpu->status_I=1;
      cycles_passed+=7;
      cpu->interrupt_pending=INTERRUPT_NONE;
    }

    // read op code
    ptOpcode = &opcode_tbl[cpu->read(cpu->reference, cpu->reg_PC)];

    // handle address mode
    switch (ptOpcode->address_mode) {
      case ADR_ABSOLUTE:
        address=READ16(cpu->reg_PC+1);
        break;
      case ADR_ABSOLUTE_X:
        address = READ16(cpu->reg_PC+1)+cpu->reg_X;
        if (PAGE_DIFFERS(address-cpu->reg_X, address)) {
          cycles_passed+=ptOpcode->page_cross_cycles;
          DUMMY_READ(address-0x100);                                            //!< dummy read
        }
        break;
      case ADR_ABSOLUTE_Y:
        address = READ16(cpu->reg_PC+1)+cpu->reg_Y;
        if (PAGE_DIFFERS(address-cpu->reg_Y, address)) {
          cycles_passed+=ptOpcode->page_cross_cycles;
          DUMMY_READ(address-0x100);                                            //!< dummy read
        }
        break;
      case ADR_ACCUMULATOR:
        DUMMY_READ(cpu->reg_PC+1);                                              // dummy read
        address=0;
        break;
      case ADR_IMMEDIATE:
        address=cpu->reg_PC+1;
        break;
      case ADR_IMPLIED:
        DUMMY_READ(cpu->reg_PC+1);                                              // dummy read
        address=0;
        break;
      case ADR_INDEXED_INDIRECT:
        address=READ16BUG((cpu->read(cpu->reference, cpu->reg_PC+1)+cpu->reg_X)&0xFF);
        break;
      case ADR_INDIRECT:
        address=READ16BUG(READ16(cpu->reg_PC+1));
        break;
      case ADR_INDIRECT_INDEXED:
        address=READ16BUG(cpu->read(cpu->reference, cpu->reg_PC+1))+cpu->reg_Y;
        if (PAGE_DIFFERS(address-cpu->reg_Y, address)) {
          cycles_passed+=ptOpcode->page_cross_cycles;
          DUMMY_READ(address-0x100);                                            // dummy read
        }
        break;
      case ADR_RELATIVE:
        address=cpu->read(cpu->reference, cpu->reg_PC+1);
        if (address<0x80) {
          address+=cpu->reg_PC+2;
        }
        else {
          address+=cpu->reg_PC+2-0x100;
        }
        break;
      case ADR_ZERO_PAGE:
        address=cpu->read(cpu->reference, cpu->reg_PC+1)&0xFF;
        break;
      case ADR_ZERO_PAGE_X:
        address=(cpu->read(cpu->reference, cpu->reg_PC+1)+cpu->reg_X)&0xFF;
        break;
      case ADR_ZERO_PAGE_Y:
        address=(cpu->read(cpu->reference, cpu->reg_PC+1)+cpu->reg_Y)&0xFF;
        break;
    }

    address&=0xFFFF; // mask 16 bit

    cpu->reg_PC+=ptOpcode->bytes; // update program counter
    cycles_passed+=ptOpcode->cycles; // update cycles for this opcode

    switch(ptOpcode->operation) {
      case OP_ADC:
        temp_value2=cpu->read(cpu->reference, address);
        temp_value=cpu->reg_A+temp_value2+cpu->status_C;
        #if JEG_USE_6502_DECIMAL_MODE == ENABLED
        if (cpu->status_D) { // bcd mode
          if (( (cpu->reg_A&0x0F)+(temp_value2&0x0F)+cpu->status_C)>9) {
            temp_value+=6;
          }
          cpu->status_V=(~(cpu->reg_A^temp_value2))&(cpu->reg_A^temp_value)&0x80?1:0;
          if (temp_value>0x99) {
            temp_value+=96;
          }
          cpu->status_C=temp_value>0x99?1:0;
        }
        else {
        #endif  
          cpu->status_C=temp_value>0xFF?1:0;
          cpu->status_V=(~(cpu->reg_A^temp_value2))&(cpu->reg_A^temp_value)&0x80?1:0;
        #if JEG_USE_6502_DECIMAL_MODE == ENABLED
        }
        #endif  
        cpu->reg_A=temp_value&0xFF;
        RECALC_ZN(cpu->reg_A);
        break;
      case OP_AND:
        cpu->reg_A=cpu->reg_A&cpu->read(cpu->reference, address);
        RECALC_ZN(cpu->reg_A);
        break;
      case OP_ASL:
        if (ptOpcode->address_mode==ADR_ACCUMULATOR) {
          cpu->status_C= cpu->reg_A&0x80?1:0;
          cpu->reg_A= (cpu->reg_A&0x7F)<<1;
          RECALC_ZN(cpu->reg_A);
        }
        else {
          temp_value=cpu->read(cpu->reference, address);
          cpu->status_C= temp_value&0x80?1:0;
          temp_value=(temp_value&0x7F)<<1;
          cpu->write(cpu->reference, address, temp_value);
          RECALC_ZN(temp_value);
        }
        break;
      case OP_BCC:
        BRANCH(!cpu->status_C);
        break;
      case OP_BCS:
        BRANCH(cpu->status_C);
        break;
      case OP_BEQ:
        BRANCH(cpu->status_Z);
        break;
      case OP_BIT:
        temp_value=cpu->read(cpu->reference, address);
        cpu->status_V=(temp_value&0x40)?1:0;
        cpu->status_Z=(temp_value&cpu->reg_A)?0:1;
        cpu->status_N=temp_value&0x80?1:0;
        break;
      case OP_BMI:
        BRANCH(cpu->status_N);
        break;
      case OP_BNE:
        BRANCH(!cpu->status_Z);
        break;
      case OP_BPL:
        BRANCH(!cpu->status_N);
        break;
      case OP_BRK:
        cpu->reg_PC++;
        PUSH(cpu->reg_PC>>8); // high byte of program counter to stack
        PUSH(cpu->reg_PC); // low byte
        PUSH(GET_FLAGS() | 0x10); // push status flags to stack
        cpu->status_I=1;
        cpu->reg_PC=READ16(0xFFFE);
        break;
      case OP_BVC:
        BRANCH(!cpu->status_V);
        break;
      case OP_BVS:
        BRANCH(cpu->status_V);
        break;
      case OP_CLC:
        cpu->status_C=0;
        break;
      case OP_CLD:
        cpu->status_D=0;
        break;
      case OP_CLI:
        cpu->status_I=0;
        break;
      case OP_CLV:
        cpu->status_V=0;
        break;
      case OP_CMP:
        COMPARE(cpu->reg_A, cpu->read(cpu->reference, address));
        break;
      case OP_CPX:
        COMPARE(cpu->reg_X, cpu->read(cpu->reference, address));
        break;
      case OP_CPY:
        COMPARE(cpu->reg_Y, cpu->read(cpu->reference, address));
        break;
      case OP_DEC:
        temp_value=cpu->read(cpu->reference, address)-1;
        if (temp_value<0) {
          temp_value=0xFF;
        }
        cpu->write(cpu->reference, address, temp_value);
        RECALC_ZN(temp_value);
        break;
      case OP_DEX:
        cpu->reg_X--;
        cpu->reg_X &= 0xFF;
        RECALC_ZN(cpu->reg_X);
        break;
      case OP_DEY:
        cpu->reg_Y--;
        cpu->reg_Y &= 0xFF;
        RECALC_ZN(cpu->reg_Y);
        break;
      case OP_EOR:
        cpu->reg_A=cpu->reg_A^cpu->read(cpu->reference, address);
        RECALC_ZN(cpu->reg_A);
        break;
      case OP_INC:
        temp_value=cpu->read(cpu->reference, address)+1;
        if (temp_value>255) {
          temp_value=0x00;
        }
        cpu->write(cpu->reference, address, temp_value);
        RECALC_ZN(temp_value);
        break;
      case OP_INX:
        cpu->reg_X++;
        cpu->reg_X &= 0xFF;
        RECALC_ZN(cpu->reg_X);
        break;
      case OP_INY:
        cpu->reg_Y++;
        cpu->reg_Y &= 0xFF;
        RECALC_ZN(cpu->reg_Y);
        break;
      case OP_JMP:
        cpu->reg_PC=address;
        break;
      case OP_JSR:
        cpu->reg_PC--;
        PUSH(cpu->reg_PC>>8); // high byte of program counter to stack
        PUSH(cpu->reg_PC); // low byte
        cpu->reg_PC=address;
        break;
      case OP_LDA:
        cpu->reg_A=cpu->read(cpu->reference, address);
        RECALC_ZN(cpu->reg_A);
        break;
      case OP_LDX:
        cpu->reg_X=cpu->read(cpu->reference, address);
        RECALC_ZN(cpu->reg_X);
        break;
      case OP_LDY:
        cpu->reg_Y=cpu->read(cpu->reference, address);
        RECALC_ZN(cpu->reg_Y);
        break;
      case OP_LSR:
        if (ptOpcode->address_mode==ADR_ACCUMULATOR) {
          cpu->status_C= cpu->reg_A&0x01?1:0;
          cpu->reg_A= cpu->reg_A>>1;
          RECALC_ZN(cpu->reg_A);
        }
        else {
          temp_value=cpu->read(cpu->reference, address);
          cpu->status_C= temp_value&0x01?1:0;
          temp_value>>=1;
          cpu->write(cpu->reference, address, temp_value);
          RECALC_ZN(temp_value);
        }
        break;
      case OP_NOP:
        break;
      case OP_ORA:
        cpu->reg_A=cpu->reg_A|cpu->read(cpu->reference, address);
        RECALC_ZN(cpu->reg_A);
        break;
      case OP_PHA:
        PUSH(cpu->reg_A);
        break;
      case OP_PHP:
        PUSH(GET_FLAGS()|0x10);
        break;
      case OP_PLA:
        cpu->reg_SP=(cpu->reg_SP+1)&0xFF;
        cpu->reg_A=cpu->read(cpu->reference, 0x100|cpu->reg_SP);
        RECALC_ZN(cpu->reg_A);
        break;
      case OP_PLP:
        cpu->reg_SP=(cpu->reg_SP+1)&0xFF;
        SET_FLAGS((cpu->read(cpu->reference, 0x100|cpu->reg_SP)&0xEF)|0x20);
        break;
      case OP_ROL:
        if (ptOpcode->address_mode==ADR_ACCUMULATOR) {
          cpu->reg_A=(cpu->reg_A<<1)+cpu->status_C;
          cpu->status_C=cpu->reg_A&0x100?1:0;
          cpu->reg_A&=0xFF;
          RECALC_ZN(cpu->reg_A);
        }
        else {
          temp_value=cpu->read(cpu->reference, address);
          temp_value=(temp_value<<1)+cpu->status_C;
          cpu->status_C=temp_value&0x100?1:0;
          temp_value&=0xFF;
          cpu->write(cpu->reference, address, temp_value);
          RECALC_ZN(temp_value);
        }
        break;
      case OP_ROR:
        if (ptOpcode->address_mode==ADR_ACCUMULATOR) {
          cpu->reg_A|=cpu->status_C<<8;
          cpu->status_C=cpu->reg_A&0x01;
          cpu->reg_A>>=1;
          RECALC_ZN(cpu->reg_A);
        }
        else {
          temp_value=cpu->read(cpu->reference, address)|(cpu->status_C<<8);
          cpu->status_C=temp_value&0x01;
          temp_value>>=1;
          cpu->write(cpu->reference, address, temp_value);
          RECALC_ZN(temp_value);
        }
        break;
      case OP_RTI:
        cpu->reg_SP=(cpu->reg_SP+1)&0xFF;
        SET_FLAGS((cpu->read(cpu->reference, 0x100|cpu->reg_SP)&0xEF)|0x20);
        cpu->reg_SP=(cpu->reg_SP+1)&0xFF;
        cpu->reg_PC=cpu->read(cpu->reference, 0x100|cpu->reg_SP);
        cpu->reg_SP=(cpu->reg_SP+1)&0xFF;
        cpu->reg_PC+=cpu->read(cpu->reference, 0x100|cpu->reg_SP)<<8;
        break;
      case OP_RTS:
        cpu->reg_SP=(cpu->reg_SP+1)&0xFF;
        cpu->reg_PC=cpu->read(cpu->reference, 0x100|cpu->reg_SP);
        cpu->reg_SP=(cpu->reg_SP+1)&0xFF;
        cpu->reg_PC+=cpu->read(cpu->reference, 0x100|cpu->reg_SP)<<8;
        cpu->reg_PC+=1;
        break;
      case OP_SBC:
        // TODO: ugly hack
        temp_value2=cpu->read(cpu->reference, address);
        temp_value=(cpu->reg_A-temp_value2-(1-cpu->status_C))&0xFFFF;
        RECALC_ZN(temp_value&0xFF);
        cpu->status_V=(cpu->reg_A^temp_value2)&(cpu->reg_A^temp_value)&0x80?1:0;
        #ifndef WITHOUT_DECIMAL_MODE
        if (cpu->status_D) { // bcd mode
          if ( ((cpu->reg_A&0x0F)-(1-cpu->status_C))<(temp_value2&0x0F)) {
            temp_value-=6;
          }
          if (temp_value>0x99) {
            temp_value-=0x60;
          }
          cpu->status_C=temp_value<0x100?1:0;
        }
        else {
        #endif
          cpu->status_C=temp_value<0x100?1:0;
        #ifndef WITHOUT_DECIMAL_MODE
        }
        #endif
        cpu->reg_A=temp_value&0xFF;
        break;
      case OP_SEC:
        cpu->status_C=1;
        break;
      case OP_SED:
        cpu->status_D=1;
        break;
      case OP_SEI:
        cpu->status_I=1;
        break;
      case OP_STA:
        cpu->write(cpu->reference, address, cpu->reg_A);
        break;
      case OP_STX:
        cpu->write(cpu->reference, address, cpu->reg_X);
        break;
      case OP_STY:
        cpu->write(cpu->reference, address, cpu->reg_Y);
        break;
      case OP_TAX:
        cpu->reg_X=cpu->reg_A;
        RECALC_ZN(cpu->reg_X);
        break;
      case OP_TAY:
        cpu->reg_Y=cpu->reg_A;
        RECALC_ZN(cpu->reg_Y);
        break;
      case OP_TSX:
        cpu->reg_X=cpu->reg_SP;
        RECALC_ZN(cpu->reg_X);
        break;
      case OP_TXA:
        cpu->reg_A=cpu->reg_X;
        RECALC_ZN(cpu->reg_A);
        break;
      case OP_TXS:
        cpu->reg_SP=cpu->reg_X;
        break;
      case OP_TYA:
        cpu->reg_A=cpu->reg_Y;
        RECALC_ZN(cpu->reg_A);
        break;
      // undocumented opcodes
      case OP_AHX:
      case OP_ALR:
      case OP_ANC:
      case OP_ARR:
      case OP_AXS:
      case OP_DCP:
      case OP_ISC:
      case OP_KIL:
      case OP_LAS:
      case OP_LAX:
      case OP_RLA:
      case OP_RRA:
      case OP_SAX:
      case OP_SHX:
      case OP_SHY:
      case OP_SLO:
      case OP_SRE:
      case OP_TAS:
      case OP_XAA:
        break;
    }

    cycles_to_run-=cycles_passed;
    cpu->cycle_number+=cycles_passed;
  } while (cycles_to_run>0);

  return cycles_passed;
}

void cpu6502_trigger_interrupt(cpu6502_t *cpu, cpu6502_interrupt_enum_t interrupt) {
  // TODO: could it happen that NMI and IRQ occuring the same time?
  switch (interrupt) {
    case INTERRUPT_NMI:
      cpu->interrupt_pending=INTERRUPT_NMI;
      break;
    case INTERRUPT_IRQ:
      if (cpu->status_I) {
        cpu->interrupt_pending=INTERRUPT_NMI;
      }
      break;
    default:
      break;
  }
}
