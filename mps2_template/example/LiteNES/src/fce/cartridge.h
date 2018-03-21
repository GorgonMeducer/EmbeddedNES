#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

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

extern uint_fast32_t cartridge_setup(   cartridge_t *cartridge, 
                                        uint8_t *data, 
                                        uint_fast32_t size);

// access cpu memory bus
extern uint_fast8_t cartridge_read_prg( cartridge_t *cartridge, 
                                        uint_fast16_t adr);
                                        
extern void cartridge_write_prg(        cartridge_t *cartridge, 
                                        uint_fast16_t adr, 
                                        uint_fast8_t value);

// access ppu memory bus
extern uint_fast8_t cartridge_read_chr( cartridge_t *cartridge, 
                                        uint_fast16_t adr);
                                        
extern void cartridge_write_chr(        cartridge_t *cartridge, 
                                        uint_fast16_t adr, 
                                        uint_fast8_t value);

#endif
