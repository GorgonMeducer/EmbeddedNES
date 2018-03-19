#include "cpu.h"
#include "cpu-internal.h"
#include "memory.h"
#include "ppu.h"
#include <string.h>
#include "app_platform.h"

//CPU_STATE cpu = {0};

NO_INIT byte CPU_RAM[0x8000];

//byte op_code;             // Current instruction code
/*
struct __opcode {
    int op_value;
    int op_address; // Arguments for current instruction
    int op_cycles;            // Additional instruction cycles used (e.g. when paging occurs)
};
*/
struct __core core;

//uint32_t cpu_cycles;  // Total CPU Cycles Since Power Up (wraps)



#define CPU_ISA_OP_BIS(o, c, f, n, a)   [0x##o] = {                             \
                            .cpu_op_cycles = c,                                 \
                            .cpu_op_handler = cpu_op_##f,                       \
                            .cpu_op_name = n,                                   \
                            .cpu_op_address_mode = cpu_address_##a,             \
                            .cpu_op_in_base_instruction_set = true,             \
                        },

// Not implemented instructions

#define CPU_ISA_OP_NII(o, a)            [0x##o] = {                             \
                            .cpu_op_cycles = 1,                                 \
                            .cpu_op_handler = ____FE____,                        \
                            .cpu_op_name = "NOP",                                \
                            .cpu_op_address_mode = cpu_address_##a,              \
                            .cpu_op_in_base_instruction_set = false,             \
                        },

// Extended instruction set found in other CPUs and implemented for compatibility

#define CPU_ISA_OP_EIS(o, c, f, n, a)   [0x##o] = {                             \
                            .cpu_op_cycles = c,                                 \
                            .cpu_op_handler = cpu_op_##f,                       \
                            .cpu_op_name = n,                                   \
                            .cpu_op_address_mode = cpu_address_##a,             \
                            .cpu_op_in_base_instruction_set = false,            \
                        },

typedef struct {
    int cpu_op_cycles;                   // CPU cycles used by instructions
    void (*cpu_op_handler)(void);            // Array of instruction function pointers
    char *cpu_op_name;                   // Instruction names
    void (*cpu_op_address_mode)(void);       // Array of address modes
    bool cpu_op_in_base_instruction_set; // true if instruction is in base 6502 instruction set
}instruction_t;



/*
int cpu_op_cycles[256];                   // CPU cycles used by instructions
void (*cpu_op_handler[256])();            // Array of instruction function pointers
char *cpu_op_name[256];                   // Instruction names
void (*cpu_op_address_mode[256])();       // Array of address modes
bool cpu_op_in_base_instruction_set[256]; // true if instruction is in base 6502 instruction set
*/

static const byte cpu_zn_flag_table[256] =
{
  zero_flag,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
  negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,negative_flag,
};

// CPU Memory
/*
inline uint_fast8_t cpu_ram_read(uint_fast16_t address)
{
    return CPU_RAM[address & 0x7FF];
}
inline uint_fast16_t cpu_ram_readw(uint_fast16_t address)
{
    return (*(uint_fast16_t *)&CPU_RAM[address & 0x7FF]);
}
void cpu_ram_write(uint_fast16_t address, uint_fast8_t data)
{
    CPU_RAM[address & 0x7FF] = data;
}

void cpu_ram_writew(uint_fast16_t address, uint_fast16_t data)
{
    (*(word *)&CPU_RAM[address & 0x7FF]) = data;
}

*/
inline byte *get_cpu_ram_address(uint_fast16_t address)
{
    return &CPU_RAM[address & 0x7FF];
}





// Interrupt Addresses

 uint_fast16_t cpu_nmi_interrupt_address(void)   { return memory_readw(0xFFFA); }
 uint_fast16_t cpu_reset_interrupt_address(void) { return memory_readw(0xFFFC); }
 uint_fast16_t cpu_irq_interrupt_address(void)   { return memory_readw(0xFFFE); }



//! \name Stack Routines
//!@{
static inline void cpu_stack_pushb(uint_fast8_t data) 
{ 
    /* bypass BUS interface to make CPU ram access faster */
    CPU_RAM[0x100 + core.SP--] = data;
    //memory_writeb(0x100 + core.SP--, data);       
}
static inline void cpu_stack_pushw(uint_fast16_t data) 
{   
    /* bypass BUS interface to make CPU ram access faster */
    (*(word *)&CPU_RAM[0xFF + core.SP]) = data;
    
    //memory_writew(0xFF + core.SP, data); 
    core.SP -= 2;
}
static inline byte cpu_stack_popb(void)       
{ 
    /* bypass BUS interface to make CPU ram access faster */
    return CPU_RAM[0x100 + ++core.SP];
    //return memory_readb(0x100 + ++core.SP);       
}
static inline uint_fast16_t cpu_stack_popw(void)       
{ 
    /* bypass BUS interface to make CPU ram access faster */
    core.SP += 2; 
    //return memory_readw(0xFF + core.SP); 
    return (*(word *)&CPU_RAM[0xFF + core.SP]);
}
//!@}


// CPU Instructions

void ____FE____() { /* Instruction for future Extension */ }

#define cpu_flag_set(flag) common_bit_set(core.P, flag)
#define cpu_modify_flag(flag, value) common_modify_bitb(&core.P, flag, value)
#define cpu_set_flag(flag) common_set_bitb(&core.P, flag)
#define cpu_unset_flag(flag) common_unset_bitb(&core.P, flag)

#define cpu_update_zn_flags(value) (core.P = (core.P & ~(zero_flag | negative_flag)) | cpu_zn_flag_table[value])

#define cpu_branch(flag) do {if (flag) core.PC = core.op_address;} while(0)
#define cpu_compare(reg)                                                        \
    do {int result = reg - core.op_value;                                       \
        cpu_modify_flag(carry_bp, result >= 0);                                 \
        cpu_modify_flag(zero_bp, result == 0);                                  \
        cpu_modify_flag(negative_bp, (result >> 7) & 1);                        \
    } while(0)



// CPU Instructions

// NOP

void cpu_op_nop(void) {}

// Addition

void cpu_op_adc(void)
{
    int result = core.A + core.op_value + (cpu_flag_set(carry_bp) ? 1 : 0);
    cpu_modify_flag(carry_bp, (result & 0x100));
    cpu_modify_flag(overflow_bp, (~(core.A ^ core.op_value) & (core.A ^ result) & 0x80));
    core.A = result & 0xFF;
    cpu_update_zn_flags(core.A);
}

// Subtraction

void cpu_op_sbc(void)
{
    int result = core.A - core.op_value - (cpu_flag_set(carry_bp) ? 0 : 1);
    cpu_modify_flag(carry_bp, !(result & 0x100));
    cpu_modify_flag(overflow_bp, !!((core.A ^ core.op_value) & (core.A ^ result) & 0x80));
    core.A = result & 0xFF;
    cpu_update_zn_flags(core.A);
}

// Bit Manipulation Operations

void cpu_op_and(void) { cpu_update_zn_flags(core.A &= core.op_value); }
void cpu_op_bit(void) { cpu_modify_flag(zero_bp, !(core.A & core.op_value)); core.P = (core.P & 0x3F) | (0xC0 & core.op_value); }
void cpu_op_eor(void) { cpu_update_zn_flags(core.A ^= core.op_value); }
void cpu_op_ora(void) { cpu_update_zn_flags(core.A |= core.op_value); }
void cpu_op_asla(void)
{
    cpu_modify_flag(carry_bp, core.A & 0x80);
    core.A <<= 1;
    cpu_update_zn_flags(core.A);
}
void cpu_op_asl(void)
{
    cpu_modify_flag(carry_bp, core.op_value & 0x80);
    core.op_value <<= 1;
    core.op_value &= 0xFF;
    cpu_update_zn_flags(core.op_value);
    memory_writeb(core.op_address, core.op_value);
}
void cpu_op_lsra(void)
{
    int value = core.A >> 1;
    cpu_modify_flag(carry_bp, core.A & 0x01);
    core.A = value & 0xFF;
    cpu_update_zn_flags(value);
}
void cpu_op_lsr(void)
{
    cpu_modify_flag(carry_bp, core.op_value & 0x01);
    core.op_value >>= 1;
    core.op_value &= 0xFF;
    memory_writeb(core.op_address, core.op_value);
    cpu_update_zn_flags(core.op_value);
}

void cpu_op_rola(void)
{
    int value = core.A << 1;
    value |= cpu_flag_set(carry_bp) ? 1 : 0;
    cpu_modify_flag(carry_bp, value > 0xFF);
    core.A = value & 0xFF;
    cpu_update_zn_flags(core.A);
}
void cpu_op_rol(void)
{
    core.op_value <<= 1;
    core.op_value |= cpu_flag_set(carry_bp) ? 1 : 0;
    cpu_modify_flag(carry_bp, core.op_value > 0xFF);
    core.op_value &= 0xFF;
    memory_writeb(core.op_address, core.op_value);
    cpu_update_zn_flags(core.op_value);
}
void cpu_op_rora(void)
{
    unsigned char carry = cpu_flag_set(carry_bp);
    cpu_modify_flag(carry_bp, core.A & 0x01);
    core.A = (core.A >> 1) | (carry << 7);
    cpu_modify_flag(zero_bp, core.A == 0);
    cpu_modify_flag(negative_bp, !!carry);
}
void cpu_op_ror(void)
{
    unsigned char carry = cpu_flag_set(carry_bp);
    cpu_modify_flag(carry_bp, core.op_value & 0x01);
    core.op_value = ((core.op_value >> 1) | (carry << 7)) & 0xFF;
    cpu_modify_flag(zero_bp, core.op_value == 0);
    cpu_modify_flag(negative_bp, !!carry);
    memory_writeb(core.op_address, core.op_value);
}

// Loading

void cpu_op_lda(void) { cpu_update_zn_flags(core.A = core.op_value); }
void cpu_op_ldx(void) { cpu_update_zn_flags(core.X = core.op_value); }
void cpu_op_ldy(void) { cpu_update_zn_flags(core.Y = core.op_value); }

// Storing

void cpu_op_sta(void) { memory_writeb(core.op_address, core.A); }
void cpu_op_stx(void) { memory_writeb(core.op_address, core.X); }
void cpu_op_sty(void) { memory_writeb(core.op_address, core.Y); }

// Transfering

void cpu_op_tax(void) { cpu_update_zn_flags(core.X = core.A);  }
void cpu_op_txa(void) { cpu_update_zn_flags(core.A = core.X);  }
void cpu_op_tay(void) { cpu_update_zn_flags(core.Y = core.A);  }
void cpu_op_tya(void) { cpu_update_zn_flags(core.A = core.Y);  }
void cpu_op_tsx(void) { cpu_update_zn_flags(core.X = core.SP); }
void cpu_op_txs(void) { core.SP = core.X; }

// Branching Positive

void cpu_op_bcs(void) { cpu_branch(cpu_flag_set(carry_bp));     }
void cpu_op_beq(void) { cpu_branch(cpu_flag_set(zero_bp));      }
void cpu_op_bmi(void) { cpu_branch(cpu_flag_set(negative_bp));  }
void cpu_op_bvs(void) { cpu_branch(cpu_flag_set(overflow_bp));  }

// Branching Negative

void cpu_op_bne(void) { cpu_branch(!cpu_flag_set(zero_bp));     }
void cpu_op_bcc(void) { cpu_branch(!cpu_flag_set(carry_bp));    }
void cpu_op_bpl(void) { cpu_branch(!cpu_flag_set(negative_bp)); }
void cpu_op_bvc(void) { cpu_branch(!cpu_flag_set(overflow_bp)); }

// Jumping

void cpu_op_jmp(void) { core.PC = core.op_address; }

// Subroutines

void cpu_op_jsr(void) { cpu_stack_pushw(core.PC - 1); core.PC = core.op_address; }
void cpu_op_rts(void) { core.PC = cpu_stack_popw() + 1; }

// Interruptions

void cpu_op_brk(void) { cpu_stack_pushw(core.PC - 1); cpu_stack_pushb(core.P); core.P |= unused_flag | break_flag; core.PC = cpu_nmi_interrupt_address(); }
void cpu_op_rti(void) { core.P = cpu_stack_popb() | unused_flag; core.PC = cpu_stack_popw(); }

// Flags

void cpu_op_clc(void) { cpu_unset_flag(carry_bp);     }
void cpu_op_cld(void) { cpu_unset_flag(decimal_bp);   }
void cpu_op_cli(void) { cpu_unset_flag(interrupt_bp); }
void cpu_op_clv(void) { cpu_unset_flag(overflow_bp);  }
void cpu_op_sec(void) { cpu_set_flag(carry_bp);       }
void cpu_op_sed(void) { cpu_set_flag(decimal_bp);     }
void cpu_op_sei(void) { cpu_set_flag(interrupt_bp);   }

// Comparison

void cpu_op_cmp(void) { cpu_compare(core.A); }
void cpu_op_cpx(void) { cpu_compare(core.X); }
void cpu_op_cpy(void) { cpu_compare(core.Y); }

// Increment

void cpu_op_inc(void) { byte result = core.op_value + 1; memory_writeb(core.op_address, result); cpu_update_zn_flags(result); }
void cpu_op_inx(void) { cpu_update_zn_flags(++core.X); }
void cpu_op_iny(void) { cpu_update_zn_flags(++core.Y); }

// Decrement

void cpu_op_dec(void) { byte result = core.op_value - 1; memory_writeb(core.op_address, result); cpu_update_zn_flags(result); }
void cpu_op_dex(void) { cpu_update_zn_flags(--core.X); }
void cpu_op_dey(void) { cpu_update_zn_flags(--core.Y); }

// Stack

void cpu_op_php(void) { cpu_stack_pushb(core.P | 0x30); }
void cpu_op_pha(void) { cpu_stack_pushb(core.A); }
void cpu_op_pla(void) { core.A = cpu_stack_popb(); cpu_update_zn_flags(core.A); }
void cpu_op_plp(void) { core.P = (cpu_stack_popb() & 0xEF) | 0x20; }



// Extended Instruction Set

void cpu_op_aso(void) { cpu_op_asl(); cpu_op_ora(); }
void cpu_op_axa(void) { memory_writeb(core.op_address, core.A & core.X & (core.op_address >> 8)); }
void cpu_op_axs(void) { memory_writeb(core.op_address, core.A & core.X); }
void cpu_op_dcm(void)
{
    core.op_value--;
    core.op_value &= 0xFF;
    memory_writeb(core.op_address, core.op_value);
    cpu_op_cmp();
}
void cpu_op_ins(void)
{
    core.op_value = (core.op_value + 1) & 0xFF;
    memory_writeb(core.op_address, core.op_value);
    cpu_op_sbc();
}
void cpu_op_lax(void) { cpu_update_zn_flags(core.A = core.X = core.op_value); }
void cpu_op_lse(void) { cpu_op_lsr(); cpu_op_eor(); }
void cpu_op_rla(void) { cpu_op_rol(); cpu_op_and(); }
void cpu_op_rra(void) { cpu_op_ror(); cpu_op_adc(); }





// Base 6502 instruction set
static const instruction_t cpu_isa[256] = {
    CPU_ISA_OP_BIS(00, 7, brk, "BRK", implied)
    CPU_ISA_OP_BIS(01, 6, ora, "ORA", indirect_x)
    CPU_ISA_OP_BIS(05, 3, ora, "ORA", zero_page)
    CPU_ISA_OP_BIS(06, 5, asl, "ASL", zero_page)
    CPU_ISA_OP_BIS(08, 3, php, "PHP", implied)
    CPU_ISA_OP_BIS(09, 2, ora, "ORA", immediate)
    CPU_ISA_OP_BIS(0A, 2, asla,"ASL", implied)
    CPU_ISA_OP_BIS(0D, 4, ora, "ORA", absolute)
    CPU_ISA_OP_BIS(0E, 6, asl, "ASL", absolute)
    CPU_ISA_OP_BIS(10, 2, bpl, "BPL", relative)
    CPU_ISA_OP_BIS(11, 5, ora, "ORA", indirect_y)
    CPU_ISA_OP_BIS(15, 4, ora, "ORA", zero_page_x)
    CPU_ISA_OP_BIS(16, 6, asl, "ASL", zero_page_x)
    CPU_ISA_OP_BIS(18, 2, clc, "CLC", implied)
    CPU_ISA_OP_BIS(19, 4, ora, "ORA", absolute_y)
    CPU_ISA_OP_BIS(1D, 4, ora, "ORA", absolute_x)
    CPU_ISA_OP_BIS(1E, 7, asl, "ASL", absolute_x)
    CPU_ISA_OP_BIS(20, 6, jsr, "JSR", absolute)
    CPU_ISA_OP_BIS(21, 6, and, "AND", indirect_x)
    CPU_ISA_OP_BIS(24, 3, bit, "BIT", zero_page)
    CPU_ISA_OP_BIS(25, 3, and, "AND", zero_page)
    CPU_ISA_OP_BIS(26, 5, rol, "ROL", zero_page)
    CPU_ISA_OP_BIS(28, 4, plp, "PLP", implied)
    CPU_ISA_OP_BIS(29, 2, and, "AND", immediate)
    CPU_ISA_OP_BIS(2A, 2, rola,"ROL", implied)
    CPU_ISA_OP_BIS(2C, 4, bit, "BIT", absolute)
    CPU_ISA_OP_BIS(2D, 2, and, "AND", absolute)
    CPU_ISA_OP_BIS(2E, 6, rol, "ROL", absolute)
    CPU_ISA_OP_BIS(30, 2, bmi, "BMI", relative)
    CPU_ISA_OP_BIS(31, 5, and, "AND", indirect_y)
    CPU_ISA_OP_BIS(35, 4, and, "AND", zero_page_x)
    CPU_ISA_OP_BIS(36, 6, rol, "ROL", zero_page_x)
    CPU_ISA_OP_BIS(38, 2, sec, "SEC", implied)
    CPU_ISA_OP_BIS(39, 4, and, "AND", absolute_y)
    CPU_ISA_OP_BIS(3D, 4, and, "AND", absolute_x)
    CPU_ISA_OP_BIS(3E, 7, rol, "ROL", absolute_x)
    CPU_ISA_OP_BIS(40, 6, rti, "RTI", implied)
    CPU_ISA_OP_BIS(41, 6, eor, "EOR", indirect_x)
    CPU_ISA_OP_BIS(45, 3, eor, "EOR", zero_page)
    CPU_ISA_OP_BIS(46, 5, lsr, "LSR", zero_page)
    CPU_ISA_OP_BIS(48, 3, pha, "PHA", implied)
    CPU_ISA_OP_BIS(49, 2, eor, "EOR", immediate)
    CPU_ISA_OP_BIS(4A, 2, lsra,"LSR", implied)
    CPU_ISA_OP_BIS(4C, 3, jmp, "JMP", absolute)
    CPU_ISA_OP_BIS(4D, 4, eor, "EOR", absolute)
    CPU_ISA_OP_BIS(4E, 6, lsr, "LSR", absolute)
    CPU_ISA_OP_BIS(50, 2, bvc, "BVC", relative)
    CPU_ISA_OP_BIS(51, 5, eor, "EOR", indirect_y)
    CPU_ISA_OP_BIS(55, 4, eor, "EOR", zero_page_x)
    CPU_ISA_OP_BIS(56, 6, lsr, "LSR", zero_page_x)
    CPU_ISA_OP_BIS(58, 2, cli, "CLI", implied)
    CPU_ISA_OP_BIS(59, 4, eor, "EOR", absolute_y)
    CPU_ISA_OP_BIS(5D, 4, eor, "EOR", absolute_x)
    CPU_ISA_OP_BIS(5E, 7, lsr, "LSR", absolute_x)
    CPU_ISA_OP_BIS(60, 6, rts, "RTS", implied)
    CPU_ISA_OP_BIS(61, 6, adc, "ADC", indirect_x)
    CPU_ISA_OP_BIS(65, 3, adc, "ADC", zero_page)
    CPU_ISA_OP_BIS(66, 5, ror, "ROR", zero_page)
    CPU_ISA_OP_BIS(68, 4, pla, "PLA", implied)
    CPU_ISA_OP_BIS(69, 2, adc, "ADC", immediate)
    CPU_ISA_OP_BIS(6A, 2, rora,"ROR", implied)
    CPU_ISA_OP_BIS(6C, 5, jmp, "JMP", indirect)
    CPU_ISA_OP_BIS(6D, 4, adc, "ADC", absolute)
    CPU_ISA_OP_BIS(6E, 6, ror, "ROR", absolute)
    CPU_ISA_OP_BIS(70, 2, bvs, "BVS", relative)
    CPU_ISA_OP_BIS(71, 5, adc, "ADC", indirect_y)
    CPU_ISA_OP_BIS(75, 4, adc, "ADC", zero_page_x)
    CPU_ISA_OP_BIS(76, 6, ror, "ROR", zero_page_x)
    CPU_ISA_OP_BIS(78, 2, sei, "SEI", implied)
    CPU_ISA_OP_BIS(79, 4, adc, "ADC", absolute_y)
    CPU_ISA_OP_BIS(7D, 4, adc, "ADC", absolute_x)
    CPU_ISA_OP_BIS(7E, 7, ror, "ROR", absolute_x)
    CPU_ISA_OP_BIS(81, 6, sta, "STA", indirect_x)
    CPU_ISA_OP_BIS(84, 3, sty, "STY", zero_page)
    CPU_ISA_OP_BIS(85, 3, sta, "STA", zero_page)
    CPU_ISA_OP_BIS(86, 3, stx, "STX", zero_page)
    CPU_ISA_OP_BIS(88, 2, dey, "DEY", implied)
    CPU_ISA_OP_BIS(8A, 2, txa, "TXA", implied)
    CPU_ISA_OP_BIS(8C, 4, sty, "STY", absolute)
    CPU_ISA_OP_BIS(8D, 4, sta, "STA", absolute)
    CPU_ISA_OP_BIS(8E, 4, stx, "STX", absolute)
    CPU_ISA_OP_BIS(90, 2, bcc, "BCC", relative)
    CPU_ISA_OP_BIS(91, 6, sta, "STA", indirect_y)
    CPU_ISA_OP_BIS(94, 4, sty, "STY", zero_page_x)
    CPU_ISA_OP_BIS(95, 4, sta, "STA", zero_page_x)
    CPU_ISA_OP_BIS(96, 4, stx, "STX", zero_page_y)
    CPU_ISA_OP_BIS(98, 2, tya, "TYA", implied)
    CPU_ISA_OP_BIS(99, 5, sta, "STA", absolute_y)
    CPU_ISA_OP_BIS(9A, 2, txs, "TXS", implied)
    CPU_ISA_OP_BIS(9D, 5, sta, "STA", absolute_x)
    CPU_ISA_OP_BIS(A0, 2, ldy, "LDY", immediate)
    CPU_ISA_OP_BIS(A1, 6, lda, "LDA", indirect_x)
    CPU_ISA_OP_BIS(A2, 2, ldx, "LDX", immediate)
    CPU_ISA_OP_BIS(A4, 3, ldy, "LDY", zero_page)
    CPU_ISA_OP_BIS(A5, 3, lda, "LDA", zero_page)
    CPU_ISA_OP_BIS(A6, 3, ldx, "LDX", zero_page)
    CPU_ISA_OP_BIS(A8, 2, tay, "TAY", implied)
    CPU_ISA_OP_BIS(A9, 2, lda, "LDA", immediate)
    CPU_ISA_OP_BIS(AA, 2, tax, "TAX", implied)
    CPU_ISA_OP_BIS(AC, 4, ldy, "LDY", absolute)
    CPU_ISA_OP_BIS(AD, 4, lda, "LDA", absolute)
    CPU_ISA_OP_BIS(AE, 4, ldx, "LDX", absolute)
    CPU_ISA_OP_BIS(B0, 2, bcs, "BCS", relative)
    CPU_ISA_OP_BIS(B1, 5, lda, "LDA", indirect_y)
    CPU_ISA_OP_BIS(B4, 4, ldy, "LDY", zero_page_x)
    CPU_ISA_OP_BIS(B5, 4, lda, "LDA", zero_page_x)
    CPU_ISA_OP_BIS(B6, 4, ldx, "LDX", zero_page_y)
    CPU_ISA_OP_BIS(B8, 2, clv, "CLV", implied)
    CPU_ISA_OP_BIS(B9, 4, lda, "LDA", absolute_y)
    CPU_ISA_OP_BIS(BA, 2, tsx, "TSX", implied)
    CPU_ISA_OP_BIS(BC, 4, ldy, "LDY", absolute_x)
    CPU_ISA_OP_BIS(BD, 4, lda, "LDA", absolute_x)
    CPU_ISA_OP_BIS(BE, 4, ldx, "LDX", absolute_y)
    CPU_ISA_OP_BIS(C0, 2, cpy, "CPY", immediate)
    CPU_ISA_OP_BIS(C1, 6, cmp, "CMP", indirect_x)
    CPU_ISA_OP_BIS(C4, 3, cpy, "CPY", zero_page)
    CPU_ISA_OP_BIS(C5, 3, cmp, "CMP", zero_page)
    CPU_ISA_OP_BIS(C6, 5, dec, "DEC", zero_page)
    CPU_ISA_OP_BIS(C8, 2, iny, "INY", implied)
    CPU_ISA_OP_BIS(C9, 2, cmp, "CMP", immediate)
    CPU_ISA_OP_BIS(CA, 2, dex, "DEX", implied)
    CPU_ISA_OP_BIS(CC, 4, cpy, "CPY", absolute)
    CPU_ISA_OP_BIS(CD, 4, cmp, "CMP", absolute)
    CPU_ISA_OP_BIS(CE, 6, dec, "DEC", absolute)
    CPU_ISA_OP_BIS(D0, 2, bne, "BNE", relative)
    CPU_ISA_OP_BIS(D1, 5, cmp, "CMP", indirect_y)
    CPU_ISA_OP_BIS(D5, 4, cmp, "CMP", zero_page_x)
    CPU_ISA_OP_BIS(D6, 6, dec, "DEC", zero_page_x)
    CPU_ISA_OP_BIS(D8, 2, cld, "CLD", implied)
    CPU_ISA_OP_BIS(D9, 4, cmp, "CMP", absolute_y)
    CPU_ISA_OP_BIS(DD, 4, cmp, "CMP", absolute_x)
    CPU_ISA_OP_BIS(DE, 7, dec, "DEC", absolute_x)
    CPU_ISA_OP_BIS(E0, 2, cpx, "CPX", immediate)
    CPU_ISA_OP_BIS(E1, 6, sbc, "SBC", indirect_x)
    CPU_ISA_OP_BIS(E4, 3, cpx, "CPX", zero_page)
    CPU_ISA_OP_BIS(E5, 3, sbc, "SBC", zero_page)
    CPU_ISA_OP_BIS(E6, 5, inc, "INC", zero_page)
    CPU_ISA_OP_BIS(E8, 2, inx, "INX", implied)
    CPU_ISA_OP_BIS(E9, 2, sbc, "SBC", immediate)
    CPU_ISA_OP_BIS(EA, 2, nop, "NOP", implied)
    CPU_ISA_OP_BIS(EC, 4, cpx, "CPX", absolute)
    CPU_ISA_OP_BIS(ED, 4, sbc, "SBC", absolute)
    CPU_ISA_OP_BIS(EE, 6, inc, "INC", absolute)
    CPU_ISA_OP_BIS(F0, 2, beq, "BEQ", relative)
    CPU_ISA_OP_BIS(F1, 5, sbc, "SBC", indirect_y)
    CPU_ISA_OP_BIS(F5, 4, sbc, "SBC", zero_page_x)
    CPU_ISA_OP_BIS(F6, 6, inc, "INC", zero_page_x)
    CPU_ISA_OP_BIS(F8, 2, sed, "SED", implied)
    CPU_ISA_OP_BIS(F9, 4, sbc, "SBC", absolute_y)
    CPU_ISA_OP_BIS(FD, 4, sbc, "SBC", absolute_x)
    CPU_ISA_OP_BIS(FE, 7, inc, "INC", absolute_x)

    CPU_ISA_OP_EIS(03, 8, aso, "SLO", indirect_x)
    CPU_ISA_OP_EIS(07, 5, aso, "SLO", zero_page)
    CPU_ISA_OP_EIS(0F, 6, aso, "SLO", absolute)
    CPU_ISA_OP_EIS(13, 8, aso, "SLO", indirect_y)
    CPU_ISA_OP_EIS(17, 6, aso, "SLO", zero_page_x)
    CPU_ISA_OP_EIS(1B, 7, aso, "SLO", absolute_y)
    CPU_ISA_OP_EIS(1F, 7, aso, "SLO", absolute_x)
    CPU_ISA_OP_EIS(23, 8, rla, "RLA", indirect_x)
    CPU_ISA_OP_EIS(27, 5, rla, "RLA", zero_page)
    CPU_ISA_OP_EIS(2F, 6, rla, "RLA", absolute)
    CPU_ISA_OP_EIS(33, 8, rla, "RLA", indirect_y)
    CPU_ISA_OP_EIS(37, 6, rla, "RLA", zero_page_x)
    CPU_ISA_OP_EIS(3B, 7, rla, "RLA", absolute_y)
    CPU_ISA_OP_EIS(3F, 7, rla, "RLA", absolute_x)
    CPU_ISA_OP_EIS(43, 8, lse, "SRE", indirect_x)
    CPU_ISA_OP_EIS(47, 5, lse, "SRE", zero_page)
    CPU_ISA_OP_EIS(4F, 6, lse, "SRE", absolute)
    CPU_ISA_OP_EIS(53, 8, lse, "SRE", indirect_y)
    CPU_ISA_OP_EIS(57, 6, lse, "SRE", zero_page_x)
    CPU_ISA_OP_EIS(5B, 7, lse, "SRE", absolute_y)
    CPU_ISA_OP_EIS(5F, 7, lse, "SRE", absolute_x)
    CPU_ISA_OP_EIS(63, 8, rra, "RRA", indirect_x)
    CPU_ISA_OP_EIS(67, 5, rra, "RRA", zero_page)
    CPU_ISA_OP_EIS(6F, 6, rra, "RRA", absolute)
    CPU_ISA_OP_EIS(73, 8, rra, "RRA", indirect_y)
    CPU_ISA_OP_EIS(77, 6, rra, "RRA", zero_page_x)
    CPU_ISA_OP_EIS(7B, 7, rra, "RRA", absolute_y)
    CPU_ISA_OP_EIS(7F, 7, rra, "RRA", absolute_x)
    CPU_ISA_OP_EIS(83, 6, axs, "SAX", indirect_x)
    CPU_ISA_OP_EIS(87, 3, axs, "SAX", zero_page)
    CPU_ISA_OP_EIS(8F, 4, axs, "SAX", absolute)
    CPU_ISA_OP_EIS(93, 6, axa, "SAX", indirect_y)
    CPU_ISA_OP_EIS(97, 4, axs, "SAX", zero_page_y)
    CPU_ISA_OP_EIS(9F, 5, axa, "SAX", absolute_y)
    CPU_ISA_OP_EIS(A3, 6, lax, "LAX", indirect_x)
    CPU_ISA_OP_EIS(A7, 3, lax, "LAX", zero_page)
    CPU_ISA_OP_EIS(AF, 4, lax, "LAX", absolute)
    CPU_ISA_OP_EIS(B3, 5, lax, "LAX", indirect_y)
    CPU_ISA_OP_EIS(B7, 4, lax, "LAX", zero_page_y)
    CPU_ISA_OP_EIS(BF, 4, lax, "LAX", absolute_y)
    CPU_ISA_OP_EIS(C3, 8, dcm, "DCP", indirect_x)
    CPU_ISA_OP_EIS(C7, 5, dcm, "DCP", zero_page)
    CPU_ISA_OP_EIS(CF, 6, dcm, "DCP", absolute)
    CPU_ISA_OP_EIS(D3, 8, dcm, "DCP", indirect_y)
    CPU_ISA_OP_EIS(D7, 6, dcm, "DCP", zero_page_x)
    CPU_ISA_OP_EIS(DB, 7, dcm, "DCP", absolute_y)
    CPU_ISA_OP_EIS(DF, 7, dcm, "DCP", absolute_x)
    CPU_ISA_OP_EIS(E3, 8, ins, "ISB", indirect_x)
    CPU_ISA_OP_EIS(E7, 5, ins, "ISB", zero_page)
    CPU_ISA_OP_EIS(EB, 2, sbc, "SBC", immediate)
    CPU_ISA_OP_EIS(EF, 6, ins, "ISB", absolute)
    CPU_ISA_OP_EIS(F3, 8, ins, "ISB", indirect_y)
    CPU_ISA_OP_EIS(F7, 6, ins, "ISB", zero_page_x)
    CPU_ISA_OP_EIS(FB, 7, ins, "ISB", absolute_y)
    CPU_ISA_OP_EIS(FF, 7, ins, "ISB", absolute_x)

    CPU_ISA_OP_NII(04, zero_page)
    CPU_ISA_OP_NII(0C, absolute)
    CPU_ISA_OP_NII(14, zero_page_x)
    CPU_ISA_OP_NII(1A, implied)
    CPU_ISA_OP_NII(1C, absolute_x)
    CPU_ISA_OP_NII(34, zero_page_x)
    CPU_ISA_OP_NII(3A, implied)
    CPU_ISA_OP_NII(3C, absolute_x)
    CPU_ISA_OP_NII(44, zero_page)
    CPU_ISA_OP_NII(54, zero_page_x)
    CPU_ISA_OP_NII(5A, implied)
    CPU_ISA_OP_NII(5C, absolute_x)
    CPU_ISA_OP_NII(64, zero_page)
    CPU_ISA_OP_NII(74, zero_page_x)
    CPU_ISA_OP_NII(7A, implied)
    CPU_ISA_OP_NII(7C, absolute_x)
    CPU_ISA_OP_NII(80, immediate)
    CPU_ISA_OP_NII(D4, zero_page_x)
    CPU_ISA_OP_NII(DA, implied)
    CPU_ISA_OP_NII(DC, absolute_x)
    CPU_ISA_OP_NII(F4, zero_page_x)
    CPU_ISA_OP_NII(FA, implied)
    CPU_ISA_OP_NII(FC, absolute_x)
};


// CPU Lifecycle

void cpu_init(void)
{
    core.P = 0x24;
    core.SP = 0x00;
    core.A = core.X = core.Y = 0;
}

void cpu_reset(void)
{
    core.PC = cpu_reset_interrupt_address();
    core.SP -= 3;
    core.P |= interrupt_flag;
}

void cpu_interrupt(void)
{
    // if (ppu_in_vblank()) {
        if (ppu_generates_nmi()) {
            core.P |= interrupt_flag;
            cpu_unset_flag(unused_bp);
            cpu_stack_pushw(core.PC);
            cpu_stack_pushb(core.P);
            core.PC = cpu_nmi_interrupt_address();
        }
    // }
}

inline uint32_t cpu_clock(void)
{
    return core.cpu_cycles;
}

void cpu_run(int_fast32_t cycles)
{   
    const instruction_t *ptIns;

    while (cycles > 0) {
        uint_fast8_t op_code = memory_readb(core.PC++);
        ptIns = &cpu_isa[op_code];
    #if 0
        if (ptIns->cpu_op_address_mode == NULL) {
            __nop();
        }
        else {
            ptIns->cpu_op_address_mode();
            ptIns->cpu_op_handler();
        }
    #else
        ptIns->cpu_op_address_mode();
        ptIns->cpu_op_handler();
    #endif
        cycles -= ptIns->cpu_op_cycles + core.op_cycles;
        core.cpu_cycles -= ptIns->cpu_op_cycles + core.op_cycles;
        core.op_cycles = 0;
    }

}

