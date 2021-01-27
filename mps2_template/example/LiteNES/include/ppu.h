#include "common.h"


#ifndef __PPU_H__
#define __PPU_H__

#include "ppu-internal.h"

extern byte PPU_SPRRAM[0x100];
extern byte PPU_RAM[0x4000];

extern PPU_STATE ppu;

extern void ppu_init(void);
extern void ppu_finish(void);


inline uint_fast8_t ppu_ram_read(uint_fast16_t address)
{
    return PPU_RAM[ppu_get_real_ram_address(address)];
}

inline void ppu_ram_write(uint_fast16_t address, uint_fast8_t data)
{
    PPU_RAM[ppu_get_real_ram_address(address)] = data;
}

extern uint_fast8_t ppu_io_read(uint_fast16_t address);
extern void ppu_io_write(uint_fast16_t address, uint_fast8_t data);


extern bool ppu_generates_nmi(void);
extern void ppu_set_generates_nmi(bool yesno);

extern void ppu_set_mirroring(byte mirroring);

extern void ppu_run(int cycles);
extern void ppu_cycle(void);
//extern int_fast32_t ppu_scanline(void);
extern void ppu_set_scanline(int s);
extern void ppu_copy(uint_fast16_t address, byte *source, int_fast32_t length);

extern void ppu_sprram_write_block(const byte *pchSrc, uint_fast32_t wSize);
inline void ppu_sprram_write(uint_fast8_t data)
{
    PPU_SPRRAM[ppu.OAMADDR++] = data;
}

// PPUCTRL
extern bool ppu_shows_background(void);
extern bool ppu_shows_sprites(void);
extern bool ppu_in_vblank(void);
extern void ppu_set_in_vblank(bool yesno);


#endif

