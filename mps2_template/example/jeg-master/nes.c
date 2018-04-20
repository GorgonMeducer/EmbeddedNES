#include "nes.h"
#include "cpu6502_debug.h"
#include <string.h>

static int cpu6502_bus_read (void *ref, int address) {
  nes_t* nes=(nes_t *)ref;
  int value;

  if (address<0x2000) {
    return nes->ram_data[address%0x800];
  }
  else if (address<0x4000) {
    return ppu_read(&nes->ppu, 0x2000+address%8);
  }
  else if (address==0x4014) {
    return ppu_read(&nes->ppu, address);
  }
  else if (address==0x4016) {
    value=nes->controller_shift_reg[0]&1;
    nes->controller_shift_reg[0]=(nes->controller_shift_reg[0]>>1)|0x80;
    return value;
  }
  else if (address==0x4017) {
    value=nes->controller_shift_reg[1]&1;
    nes->controller_shift_reg[1]=(nes->controller_shift_reg[1]>>1)|0x80;
    return value;
  }
  else if (address>=0x6000) {
    return cartridge_read_prg(&nes->cartridge, address);
  }
  else {
    // TODO: log this event
    return 0;
  }
}

static void cpu6502_bus_write (void *ref, int address, int value) {
  nes_t* nes=(nes_t *)ref;

  if (address<0x2000) {
    nes->ram_data[address%0x800]=value;
  }
  else if (address<0x4000) {
    ppu_write(&nes->ppu, 0x2000+address%8, value);
  }
  else if (address==0x4014) {
    ppu_write(&nes->ppu, address, value);
  }
  else if (address==0x4016 && value&0x01) {
    nes->controller_shift_reg[0]=nes->controller_data[0];
    nes->controller_shift_reg[1]=nes->controller_data[1];
  }
  else if (address>=0x6000) {
    cartridge_write_prg(&nes->cartridge, address, value);
  }
  else {
    // TODO: log this event
  }
}

const int mirror_lookup[20]={0,0,1,1,0,1,0,1,0,0,0,0,1,1,1,1,0,1,2,3};

int mirror_address (int mode, int address) {
  address=address%0x1000;
  return 0x2000+mirror_lookup[mode*4+(address/0x400)]*0x400+(address%0x400);
}

static int ppu_bus_read (nes_t *nes, int address) {
  int value;
  address%=0x4000;
  if (address<0x2000) {
    value=cartridge_read_chr(&nes->cartridge, address);
  }
  else if (address<0x3F00) {
    value=nes->ppu.name_table[mirror_address(nes->cartridge.mirror, address)%2048];
  }
  else if (address<0x4000) {
    address=address%32;
    if (address>=16 && address%4==0) {
      address-=16;
    }
    value=nes->ppu.palette[address];
  }
  return value;
}

static void ppu_bus_write (nes_t *nes, int address, int value) {
  address%=0x4000;
  if (address<0x2000) {
    cartridge_write_chr(&nes->cartridge, address, value);
  }
  else if (address<0x3F00) {
    nes->ppu.name_table[mirror_address(nes->cartridge.mirror, address)%2048]=value;
  }
  else if (address<0x4000) {
    address=address%32;
    if (address>=16 && address%4==0) {
      address-=16;
    }
    nes->ppu.palette[address]=value;
  }
}

void nes_init(nes_t *nes) { 
  cpu6502_init(&nes->cpu, nes, cpu6502_bus_read, cpu6502_bus_write);
  ppu_init(&nes->ppu, nes, ppu_bus_read, ppu_bus_write);
}

int nes_setup_rom(nes_t *nes, uint8_t *data, uint32_t size) {
  int result;

  nes->controller_data[0]=0;
  nes->controller_data[1]=0;
  nes->controller_shift_reg[0]=0;
  nes->controller_shift_reg[1]=0;

  result=cartridge_setup(&nes->cartridge, data, size);
  if (result==0) {
    nes_reset(nes);
  }
  return result;
}

void nes_setup_video(nes_t *nes, uint8_t *video_frame_data) {
  ppu_setup_video(&nes->ppu, video_frame_data);
}

void nes_reset(nes_t *nes) {
  cpu6502_reset(&nes->cpu);
  ppu_reset(&nes->ppu);
  memset(&nes->ram_data, 0, 0x800);
}

void nes_iterate_frame(nes_t *nes) {
  cpu6502_run(&nes->cpu, ppu_update(&nes->ppu));
}

void nes_set_controller(nes_t *nes, uint8_t controller1, uint8_t controller2) {
  nes->controller_data[0]=controller1;
  nes->controller_data[1]=controller2;
}
