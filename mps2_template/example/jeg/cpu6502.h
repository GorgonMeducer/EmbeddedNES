#ifndef CPU6502_H
#define CPU6502_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {INTERRUPT_NONE=0, INTERRUPT_NMI, INTERRUPT_IRQ} cpu6502_interrupt_enum_t;

typedef uint_fast8_t (cpu6502_read_func_t) (void *, uint_fast16_t hwAddress); // read data [8bit] from address [16bit]
typedef void (cpu6502_write_func_t) (void *, uint_fast16_t hwAddress, uint_fast8_t chValue); // write data [8bit] to address [16bit]

typedef uint_fast16_t (cpu6502_readw_func_t) (void *, uint_fast16_t hwAddress);
typedef void (cpu6502_writew_func_t)(void *, uint_fast16_t hwAddress, uint_fast16_t hwValue);

typedef struct cpu6502_t {
  // internal registers
  int reg_A; // accumulator [8Bit]
  int reg_X; // x register [8Bit]
  int reg_Y; // y register [8Bit]
  int reg_SP; // stack pointer [8Bit]
  int reg_PC; // program counter [16Bit]

  // status flags
  int status_C; // carry flag [Bit0]
  int status_Z; // zero flag [Bit1]
  int status_I; // interrupt flag [Bit2]
  int status_D; // decimal mode flag [Bit3]
  int status_B; // break command flag [Bit4]
  int status_U; // unused flag [Bit5]
  int status_V; // overflow flag [Bit6]
  int status_N; // negative flag [Bit7]

  // emulation internals
  uint64_t cycle_number; // number of actual cycle (measured in ppu cycles)
  int stall_cycles; // number of stall cycles
  cpu6502_interrupt_enum_t interrupt_pending; // type of pending interrupt

  // memory interface
  void *reference; // pointer to a reference, added as argument to read and write functions
  cpu6502_read_func_t *read;
  cpu6502_write_func_t *write;
  cpu6502_readw_func_t *readw;
  cpu6502_writew_func_t *writew;
} cpu6502_t;

typedef struct 
{
    void                    *reference; 
    cpu6502_read_func_t     *read; 
    cpu6502_write_func_t    *write;
    cpu6502_readw_func_t    *readw; 
    cpu6502_writew_func_t   *writew;
}cpu6502_cfg_t;

extern bool cpu6502_init(cpu6502_t *cpu, cpu6502_cfg_t *ptCFG);

extern void cpu6502_reset(cpu6502_t *cpu); // reset cpu to powerup state
extern int cpu6502_run(cpu6502_t *cpu, int n_cycles); // run cpu for (at least) n_cycles; a started instruction will not be "truncated";
                                               // returns number of cycles cpu ran
extern void cpu6502_trigger_interrupt(cpu6502_t *cpu, cpu6502_interrupt_enum_t interrupt); // trigger an interrupt

#endif
