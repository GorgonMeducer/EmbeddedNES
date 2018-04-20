#ifndef NES_H
#define NES_H

#include <stdint.h>

#include "ppu.h"
#include "cpu6502.h"
#include "cartridge.h"

typedef struct nes_t {
  cpu6502_t cpu;
  ppu_t ppu;
  cartridge_t cartridge;
  uint8_t controller_data[2];
  uint8_t controller_shift_reg[2];
  uint8_t ram_data[0x800];
} nes_t;

void nes_init(nes_t *nes);
int nes_setup_rom(nes_t *nes, uint8_t *ines_data, uint32_t ines_size);

void nes_setup_video(nes_t *nes, uint8_t *video_frame_data);

void nes_reset(nes_t *nes);

void nes_iterate_frame(nes_t *nes); // run cpu until next complete frame
void nes_set_controller(nes_t *nes, uint8_t controller1, uint8_t controller2); // [7:Right, 6:Left, 5:Down, 4:Up, 3:Start, 2:Select, 1:B, 0:A]

#endif
