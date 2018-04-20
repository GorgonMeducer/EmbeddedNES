#include "cartridge.h"
#include <stdio.h>

int cartridge_setup(cartridge_t *cartridge, uint8_t *data, uint32_t size) {
  // check minimum size (header is 16 bytes)
  if (size<16) {
    return 1;
  }

  // check header to be "NES\x1A"
  if (data[0]!='N' || data[1]!='E' || data[2]!='S' || data[3]!=0x1A) {
    return 2;
  }

  cartridge->prg_size=16384*data[4];
  cartridge->chr_size=8192*data[5];
  cartridge->mirror=(data[6]&0x01)+((data[6]>>2)&0x02);

  if (data[7]=='D' && data[8]=='i' && data[9]=='s' && data[10]=='k' && data[11]=='D' &&
      data[12]=='u' && data[13]=='d' && data[14]=='e' && data[15]=='!') {
    // if "DiskDude!" string is present, ignore upper 4 bits for mapper
    cartridge->mapper=(data[6]>>4);
  }
  else {
    cartridge->mapper=(data[6]>>4)+(data[7]&0xF0);
  }

  if (cartridge->mapper!=0 && cartridge->mapper!=3) { // ines #3 is just a quick fix for a test rom
    printf("%d\n", cartridge->mapper);
    return 3;
  }

  if (size<cartridge->prg_size+cartridge->chr_size+16+(data[6]&0x04?512:0)) {
    return 4;
  }

  for (int i=0; i<0x2000; i++) {
    cartridge->io_data[i]=0;
  }

  cartridge->prg_memory=data+(data[6]&0x04?512:0)+16; // skip header and trainer data
  if (cartridge->chr_size) {
    cartridge->chr_memory=cartridge->prg_memory+cartridge->prg_size;
  }
  else {
    cartridge->chr_memory=cartridge->chr_data;
    cartridge->chr_size=0x2000;
    for (int i=0; i<0x2000; i++) {
      cartridge->chr_memory[i]=0;
    }
  }

  return 0;
}


// access cpu memory bus
int cartridge_read_prg(cartridge_t *cartridge, int adr) {
  if (adr>=0x8000) {
    return cartridge->prg_memory[(adr-0x8000)%cartridge->prg_size];
  }
  else {
    return cartridge->io_data[adr-0x6000];
  }
}

void cartridge_write_prg(cartridge_t *cartridge, int adr, int value) {
  if (adr>=0x8000) {
    cartridge->prg_memory[(adr-0x8000)%cartridge->prg_size]=value;
  }
  else {
    cartridge->io_data[adr-0x6000]=value;
  }
}

// access ppu memory bus
int cartridge_read_chr(cartridge_t *cartridge, int adr) {
  return cartridge->chr_memory[adr%cartridge->chr_size];
}

void cartridge_write_chr(cartridge_t *cartridge, int adr, int value) {
  cartridge->chr_memory[adr%cartridge->chr_size]=value;
}
