#ifndef CPU6502_H
#define CPU6502_H

#include <stdint.h>

typedef enum {INTERRUPT_NONE=0, INTERRUPT_NMI, INTERRUPT_IRQ} cpu6502_interrupt_enum_t;

typedef int (*cpu6502_read_func_t) (void *, int address); // read data [8bit] from address [16bit]
typedef void (*cpu6502_write_func_t) (void *, int address, int value); // write data [8bit] to address [16bit]

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
  cpu6502_read_func_t read;
  cpu6502_write_func_t write;
} cpu6502_t;

void cpu6502_init(cpu6502_t *cpu, void *reference, cpu6502_read_func_t read, cpu6502_write_func_t write);

void cpu6502_reset(cpu6502_t *cpu); // reset cpu to powerup state
int cpu6502_run(cpu6502_t *cpu, int n_cycles); // run cpu for (at least) n_cycles; a started instruction will not be "truncated";
                                               // returns number of cycles cpu ran
void cpu6502_trigger_interrupt(cpu6502_t *cpu, cpu6502_interrupt_enum_t interrupt); // trigger an interrupt

#endif
