#include "fce.h"
#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "hal.h"
#include "nes.h"
#include <string.h>

NO_INIT PixelBuf bg, bbg, fg;

typedef struct {
    uint8_t signature[4];
    uint8_t prg_block_count;
    uint8_t chr_block_count;
    uint16_t rom_type;
    uint8_t reserved[8];
} ines_header;

static ines_header fce_rom_header;

// FCE Lifecycle

void romread(char *rom, void *buf, int size)
{
    static int off = 0;
    memcpy(buf, rom + off, size);
    off += size;
}

#define LOAD_ROM(__DES, __SIZE)                 \
        do {                                    \
            uint32_t wSize = (__SIZE);          \
            memcpy(__DES, pchRom, wSize);       \
            pchRom += wSize;                    \
        } while(0)
        

int fce_load_rom(char *rom)
{
    uint8_t *pchRom = rom;
    //romread(rom, &fce_rom_header, sizeof(fce_rom_header));
    LOAD_ROM(&fce_rom_header, sizeof(fce_rom_header));

    if (memcmp(fce_rom_header.signature, "NES\x1A", 4)) {
        return -1;
    }

    mmc_id = ((fce_rom_header.rom_type & 0xF0) >> 4);

    int prg_size = fce_rom_header.prg_block_count * 0x4000;

    if (mmc_id == 0 || mmc_id == 3) {
        // if there is only one PRG block, we must repeat it twice
        if (fce_rom_header.prg_block_count == 1) {
            mmc_copy(0x8000, pchRom, 0x4000);
            mmc_copy(0xC000, pchRom, 0x4000);
        } else {
            mmc_copy(0x8000, pchRom, 0x8000);
        }
        pchRom += prg_size;
    } else {
        return -1;
    }

    // Copying CHR pages into MMC and PPU
    int i;
    for (i = 0; i < fce_rom_header.chr_block_count; i++) {
        mmc_append_chr_rom_page(pchRom);

        if (i == 0) {
            ppu_copy(0x0000, pchRom, 0x2000);
        }
        
        pchRom += 0x2000;
    }

    return 0;
}

void fce_init(void)
{
    nes_hal_init();
    cpu_init();
    ppu_init();
    ppu_set_mirroring(fce_rom_header.rom_type & 1);
    cpu_reset();
}

extern void wait_for_frame(void);

void fce_run(void)
{
    while(1)
    {
        wait_for_frame();
        int scanlines = 262;
        while (scanlines-- > 0) {
            ppu_run(1);
            cpu_run(1364 / 12); // 1 scanline
        }
    }
}

// Rendering


void fce_update_screen(void)
{
    int idx = ppu_ram_read(0x3F00);
    nes_set_bg_color(idx);
    
    if (ppu_shows_sprites()) {
        nes_flush_buf(&bbg);
    }
    
    if (ppu_shows_background()) {
        nes_flush_buf(&bg);
    }
    
    if (ppu_shows_sprites()) {
        nes_flush_buf(&fg);
    }
    
    nes_flip_display();

    pixbuf_clean(bbg);
    pixbuf_clean(bg);
    pixbuf_clean(fg);
}

