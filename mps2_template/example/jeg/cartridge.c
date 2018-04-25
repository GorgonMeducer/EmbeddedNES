#include "cartridge.h"
#include <string.h>

int_fast32_t cartridge_setup(cartridge_t *cartridge, uint8_t *data, uint_fast32_t size) {
    uint_fast32_t prg_size, chr_size;

    // check minimum size (header is 16 bytes)
    if (size<16) {
        return 1;
    }

    if (strncmp("NES\x1A", (const char *)&data[0], 4)) {
        return 2;
    }

    prg_size = 0x4000 * data[4];
    chr_size = 0x2000 * data[5];
    cartridge->mirror = (data[6]&0x01)+((data[6]>>2)&0x02);

    if (strncmp("DiskDude!", (const char *)&data[7], 9)) {
        //! if "DiskDude!" string is present, ignore upper 4 bits for mapper
        cartridge->mapper = (data[6] >> 4);
    } else {
        cartridge->mapper = (data[6] >> 4) + (data[7] & 0xF0);
    }


    if (cartridge->mapper!=0 && cartridge->mapper!=3) { // ines #3 is just a quick fix for a test rom
        return 3;
    }

    if (size < (    prg_size 
                +   chr_size 
                +   16 
                +   ( (data[6] & 0x04) ? 512 : 0 ))) {
        return 4;
    }

    memset(cartridge->io_data, 0, 0x2000);

    //! skip header and trainer data
    cartridge->prg_memory =     data 
                            +   ((data[6] & 0x04) ? 512 : 0)
                            +   16; 
    
    if (chr_size) {
        cartridge->chr_memory = cartridge->prg_memory + prg_size;
        
    } else {
        cartridge->chr_memory = cartridge->chr_data;
        chr_size = 0x2000;

        memset(cartridge->chr_memory, 0, 0x2000);
    }

    //! generate mask
    cartridge->chr_adr_mask = chr_size - 1;
    cartridge->prg_adr_mask = prg_size - 1;
    
    return 0;
}

#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED
uint8_t *cartridge_get_prg_src_address(cartridge_t *cartridge, uint_fast16_t hwAddress)
{
    if (hwAddress>=0x8000) {
        return &(cartridge->prg_memory[ hwAddress & cartridge->prg_adr_mask]);
    } 
    return  &(cartridge->io_data[hwAddress & 0x1fff]);
}
#endif

//! \brief access cpu memory bus
uint_fast8_t cartridge_read_prg(cartridge_t *cartridge, uint_fast16_t adr) {

    if (adr >= 0x8000) {
        return cartridge->prg_memory[adr & cartridge->prg_adr_mask];
    }    
    
    return cartridge->io_data[adr & 0x1FFF];
}

#if JEG_USE_EXTRA_16BIT_BUS_ACCESS       == ENABLED
uint_fast16_t cartridge_readw_prg(cartridge_t *cartridge, uint_fast16_t adr)
{
    if (adr >= 0x8000) {
        return *(uint16_t *)&(cartridge->prg_memory[adr & cartridge->prg_adr_mask]);
    }    
    
    return cartridge->io_data[adr & 0x1FFF];
}
#endif

void cartridge_write_prg(cartridge_t *cartridge, uint_fast16_t adr, uint_fast8_t value) 
{

    if (adr >= 0x8000) {
        cartridge->prg_memory[ adr & cartridge->prg_adr_mask] = value;
    } else {
        cartridge->io_data[adr & 0x1fff] = value;
    }
  
}

//! \brief access ppu memory bus
uint_fast8_t cartridge_read_chr(cartridge_t *cartridge, uint_fast16_t adr) 
{
    return cartridge->chr_memory[adr & cartridge->chr_adr_mask];
}

void cartridge_write_chr(cartridge_t *cartridge, uint_fast16_t adr, uint_fast8_t value) 
{
    cartridge->chr_memory[adr & cartridge->chr_adr_mask ] = value;
}