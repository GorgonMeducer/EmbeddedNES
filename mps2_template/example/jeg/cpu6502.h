#ifndef CPU6502_H
#define CPU6502_H

#include "common.h"
#include "jeg_cfg.h"

typedef enum {INTERRUPT_NONE=0, INTERRUPT_NMI, INTERRUPT_IRQ} cpu6502_interrupt_enum_t;

typedef uint_fast8_t cpu6502_read_func_t (void *, uint_fast16_t hwAddress); // read data [8bit] from address [16bit]
typedef void cpu6502_write_func_t (void *, uint_fast16_t hwAddress, uint_fast8_t chValue); // write data [8bit] to address [16bit]

#if JEG_USE_EXTRA_16BIT_BUS_ACCESS == ENABLED
typedef uint_fast16_t cpu6502_readw_func_t (void *, uint_fast16_t hwAddress);
typedef void cpu6502_writew_func_t(void *, uint_fast16_t hwAddress, uint_fast16_t hwValue);
#endif

#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED
typedef uint8_t *cpu6502_dma_get_src_addr(void *, uint_fast16_t hwAddress);
#endif

typedef struct cpu6502_t {
    // internal registers (16bit is needed for correct overflow handling)
    int_fast16_t reg_A; // accumulator [8Bit]
    int_fast16_t reg_X; // x register [8Bit]
    int_fast16_t reg_Y; // y register [8Bit]
    int_fast16_t reg_SP; // stack pointer [8Bit]
    uint_fast16_t reg_PC; // program counter [16Bit]

    // status flags
    union {
        struct {
            uint8_t status_C : 1; // carry flag [Bit0]
            uint8_t status_Z : 1; // zero flag [Bit1]
            uint8_t status_I : 1; // interrupt flag [Bit2]
            uint8_t status_D : 1; // decimal mode flag [Bit3]
            uint8_t status_B : 1; // break command flag [Bit4]
            uint8_t status_U : 1; // unused flag [Bit5]
            uint8_t status_V : 1; // overflow flag [Bit6]
            uint8_t status_N : 1; // negative flag [Bit7]
        };
        uint8_t chStatus;
    };
    // emulation internals
    uint64_t cycle_number; // number of actual cycle (measured in ppu cycles)
    int_fast32_t stall_cycles; // number of stall cycles
    cpu6502_interrupt_enum_t interrupt_pending; // type of pending interrupt

    // memory interface
    void *reference; // pointer to a reference, added as argument to read and write functions
    cpu6502_read_func_t *read;
    cpu6502_write_func_t *write;
#if JEG_USE_EXTRA_16BIT_BUS_ACCESS == ENABLED
    cpu6502_readw_func_t *readw;
    cpu6502_writew_func_t *writew;
#endif
#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED
    cpu6502_dma_get_src_addr *fnDMAGetSourceAddress;
#endif
} cpu6502_t;

#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED ||                          \
    JEG_USE_EXTRA_16BIT_BUS_ACCESS       == ENABLED
typedef struct 
{
    void                    *reference; 
    cpu6502_read_func_t     *read; 
    cpu6502_write_func_t    *write;
#if JEG_USE_EXTRA_16BIT_BUS_ACCESS == ENABLED
    cpu6502_readw_func_t    *readw; 
    cpu6502_writew_func_t   *writew;
#endif
#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED
    cpu6502_dma_get_src_addr *fnDMAGetSourceAddress;
#endif
}cpu6502_cfg_t;

extern bool cpu6502_init(cpu6502_t *cpu, cpu6502_cfg_t *ptCFG);
#else
extern void cpu6502_init(cpu6502_t *cpu, void *reference, cpu6502_read_func_t read, cpu6502_write_func_t write);
#endif

extern void cpu6502_reset(cpu6502_t *cpu); // reset cpu to powerup state

//! \brief run cpu for (at least) n_cycles; a started instruction will not be "truncated";
extern uint_fast32_t cpu6502_run(cpu6502_t *cpu, int_fast32_t n_cycles); 
                                               // returns number of cycles cpu ran
extern void cpu6502_trigger_interrupt(cpu6502_t *cpu, cpu6502_interrupt_enum_t interrupt); // trigger an interrupt

#endif
