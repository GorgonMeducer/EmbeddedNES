#include "mmc.h"
#include "ppu.h"
#include <string.h>
#include "app_platform.h"

#ifndef MMC_MAX_PAGE_COUNT
#   warning No defined MMC_MAX_PAGE_COUNT, default 4 is used. 
#   define MMC_MAX_PAGE_COUNT               (4)
#endif

byte mmc_id;

//NO_INIT byte mmc_prg_pages[MMC_MAX_PAGE_COUNT][0x4000];
NO_INIT byte mmc_chr_pages[MMC_MAX_PAGE_COUNT][0x2000];
int mmc_prg_pages_number, mmc_chr_pages_number;

NO_INIT byte memory[0x10000];
/*
inline uint_fast8_t mmc_read(uint_fast16_t address)
{
    return memory[address];
}

inline uint_fast16_t mmc_readw(uint_fast16_t address)
{
    return (*(uint_fast16_t *)&memory[address]);
}
*/
inline void mmc_write(uint_fast16_t address, uint_fast8_t data)
{
    switch (mmc_id) {
        case 0x3: {
            ppu_copy(0x0000, &mmc_chr_pages[data & 3][0], 0x2000);
        }
        break;
    }
    memory[address] = data;
}

inline void mmc_writew(uint_fast16_t address, uint_fast8_t data)
{
    switch (mmc_id) {
        case 0x3: {
            ppu_copy(0x0000, &mmc_chr_pages[data & 3][0], 0x2000);
        }
        break;
    }
    (*(word *)&memory[address]) = data;
}

inline void mmc_copy(uint_fast16_t address, byte *source, uint_fast32_t length)
{
    memcpy(&memory[address], source, length);
}

inline void mmc_append_chr_rom_page(byte *source)
{
    memcpy(&mmc_chr_pages[mmc_chr_pages_number++][0], source, 0x2000);
}
