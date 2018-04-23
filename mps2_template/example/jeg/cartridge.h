#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>

typedef struct cartridge_t {
  int prg_size;
  uint8_t *prg_memory;
  int chr_size;
  uint8_t *chr_memory;
  uint8_t ram_data[0x2000];
  uint8_t io_data[0x2000];
  uint8_t chr_data[0x2000];
  int mapper;
  int mirror; // 0-horizontal, 1-vertical, 2-none
} cartridge_t;

int cartridge_setup(cartridge_t *cartridge, uint8_t *data, uint32_t size);

// access cpu memory bus
extern uint_fast8_t cartridge_read_prg(cartridge_t *cartridge, uint_fast16_t adr);
extern uint_fast16_t cartridge_readw_prg(cartridge_t *cartridge, uint_fast16_t adr);

extern void cartridge_write_prg(cartridge_t *cartridge, int adr, int value);
extern uint8_t *cartridge_get_prg_src_address(cartridge_t *cartridge, uint_fast16_t hwAddress);

// access ppu memory bus
int cartridge_read_chr(cartridge_t *cartridge, int adr);
void cartridge_write_chr(cartridge_t *cartridge, int adr, int value);

#endif