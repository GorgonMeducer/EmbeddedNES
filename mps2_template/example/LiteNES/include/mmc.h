#ifndef __MMC_H__
#include "common.h"

extern byte mmc_id;

extern byte memory[0x10000];

inline uint_fast8_t mmc_read(uint_fast16_t address)
{
    return memory[address];
}
inline uint_fast16_t mmc_readw(uint_fast16_t address)
{
    return (*(word *)&memory[address]);
}

extern void mmc_write(uint_fast16_t address, uint_fast8_t data);
extern void mmc_writew(uint_fast16_t address, uint_fast8_t data);

void mmc_copy(uint_fast16_t address, byte *source, uint_fast32_t length);
void mmc_append_chr_rom_page(byte *source);

#endif
