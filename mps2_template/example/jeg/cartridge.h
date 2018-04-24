#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include "jeg_cfg.h"

typedef struct {
  uint_fast32_t     prg_adr_mask;
  uint8_t          *prg_memory;
  uint_fast32_t     chr_adr_mask;
  uint8_t          *chr_memory;
  uint8_t           ram_data[0x2000];
  uint8_t           io_data[0x2000];
  uint8_t           chr_data[0x2000];
  int_fast32_t      mapper;
  int_fast32_t      mirror; // 0-horizontal, 1-vertical, 2-none
} cartridge_t;

extern int_fast32_t cartridge_setup(cartridge_t *cartridge, uint8_t *data, uint_fast32_t size);

// access cpu memory bus
extern uint_fast8_t cartridge_read_prg(cartridge_t *cartridge, uint_fast16_t adr);
extern uint_fast8_t cartridge_read_chr(cartridge_t *cartridge, uint_fast16_t adr);


#if JEG_USE_EXTRA_16BIT_BUS_ACCESS       == ENABLED
extern uint_fast16_t cartridge_readw_prg(cartridge_t *cartridge, uint_fast16_t adr);
#endif

#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED
extern uint8_t *cartridge_get_prg_src_address(cartridge_t *cartridge, uint_fast16_t hwAddress);
#endif

// access ppu memory bus

extern void cartridge_write_prg(cartridge_t *cartridge, uint_fast16_t adr, uint_fast8_t value);
extern void cartridge_write_chr(cartridge_t *cartridge, uint_fast16_t adr, uint_fast8_t value);

#endif
