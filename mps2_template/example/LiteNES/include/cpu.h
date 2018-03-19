#include "common.h"

#ifndef __CPU_H__
#define __CPU_H__

extern byte CPU_RAM[0x8000];

inline uint_fast8_t cpu_ram_read(uint_fast16_t address)
{
    return CPU_RAM[address & 0x7FF];
}

inline uint_fast16_t cpu_ram_readw(uint_fast16_t address)
{
    return (*(word *)&CPU_RAM[address & 0x7FF]);
}

inline void cpu_ram_write(uint_fast16_t address, uint_fast8_t data)
{
    CPU_RAM[address & 0x7FF] = data;
}

inline void cpu_ram_writew(uint_fast16_t address, uint_fast16_t data)
{
    (*(word *)&CPU_RAM[address & 0x7FF]) = data;
}


extern byte *get_cpu_ram_address(uint_fast16_t address);

extern void cpu_init(void);
extern void cpu_reset(void);
extern void cpu_interrupt(void);
extern void cpu_run(int_fast32_t cycles);

// CPU cycles that passed since power up
uint32_t cpu_clock(void);

#endif
