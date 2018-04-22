#ifndef NES_H
#define NES_H

#include <stdint.h>
#include <stdbool.h>
#include "ppu.h"
#include "cpu6502.h"
#include "cartridge.h"

typedef struct {
    ppu_draw_pixel_func_t *fnDrawPixel;
    void *ptTag;
}nes_cfg_t;

typedef struct nes_t {
  cpu6502_t cpu;
  ppu_t ppu;
  cartridge_t cartridge;
  uint8_t controller_data[2];
  uint8_t controller_shift_reg[2];
  uint8_t ram_data[0x800];
} nes_t;

extern bool nes_init(nes_t *, nes_cfg_t *);
extern int nes_setup_rom(nes_t *, uint8_t *, uint32_t );

//extern void nes_setup_video(nes_t *, uint8_t *);

extern void nes_reset(nes_t *);

extern void nes_iterate_frame(nes_t *); // run cpu until next complete frame
extern void nes_set_controller(nes_t *, uint8_t , uint8_t ); // [7:Right, 6:Left, 5:Down, 4:Up, 3:Start, 2:Select, 1:B, 0:A]
extern bool nes_is_ready_to_refresh(nes_t *nes);
#endif
