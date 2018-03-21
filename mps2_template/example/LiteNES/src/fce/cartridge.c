#include "cartridge.h"
#include <stdio.h>
#include <string.h>

uint_fast32_t cartridge_setup(cartridge_t *cartridge, uint8_t *pchROM, uint_fast32_t size) 
{
    /* check minimum size (header is 16 bytes) */
    if ( size < 16 ) {
        return 1;
    }

    /* check header to be "NES\x1A" */
    if (    pchROM[0]!='N' 
        ||  pchROM[1]!='E' 
        ||  pchROM[2]!='S' 
        ||  pchROM[3]!=0x1A) {
        return 2;
    }

    cartridge->prg_size = 0x4000 * pchROM[4];
    cartridge->chr_size = 0x2000 * pchROM[5];
    cartridge->mirror = (pchROM[6]&0x01)+((pchROM[6]>>2)&0x02);

    if (    pchROM[7]=='D' 
        &&  pchROM[8]=='i' 
        &&  pchROM[9]=='s' 
        &&  pchROM[10]=='k' 
        &&  pchROM[11]=='D' 
        &&  pchROM[12]=='u' 
        &&  pchROM[13]=='d' 
        &&  pchROM[14]=='e' 
        &&  pchROM[15]=='!') {
        /* if "DiskDude!" string is present, ignore upper 4 bits for mapper */
        cartridge->mapper=( pchROM[6]>>4 );
    } else {
        cartridge->mapper=( pchROM[6]>>4 ) + ( pchROM[7] & 0xF0 );
    }

    /*  At the moment only one mapper is working 
        (INES #0) which is running simple games like Super Mario Bros.
                                                -- Günther
     */
    if (cartridge->mapper!=0) {
        return 3;
    }

    if (size <  (   cartridge->prg_size 
                +   cartridge->chr_size 
                +   16 
                +   ((pchROM[6] & 0x04) ? 512:0 ))) {
        return 4;
    }
 
    /*! I don't think it is necessary to initialise io_data, but just in case */
    memset(cartridge->io_data, 0, 0x2000);
    
    // skip header and trainer pchROM
    cartridge->prg_memory =     pchROM 
                            +   ((pchROM[6] & 0x04) ? 512:0) 
                            +   16; 
    
    
    
    if (cartridge->chr_size) {
        cartridge->chr_memory = cartridge->prg_memory + cartridge->prg_size;
    } else {
        cartridge->chr_memory = cartridge->chr_data;
        cartridge->chr_size = 0x2000;
        
        /*! I don't think it is necessary to initialise chr_memory, 
         *  but just in case
         */
        memset(cartridge->chr_memory, 0, 0x2000);
    }

    return 0;
}


// access cpu memory bus
uint_fast8_t cartridge_read_prg(cartridge_t *cartridge, uint_fast16_t adr) 
{
    if (adr>=0x8000) {
        return cartridge->prg_memory[(adr-0x8000)%cartridge->prg_size];
    } 
    
    return cartridge->io_data[adr-0x6000];
}

void cartridge_write_prg(cartridge_t *cartridge, uint_fast16_t adr, uint_fast8_t value) 
{
    if (adr>=0x8000) {
        cartridge->prg_memory[(adr-0x8000)%cartridge->prg_size]=value;
    } else {
        cartridge->io_data[adr-0x6000]=value;
    }
}

// access ppu memory bus
uint_fast8_t cartridge_read_chr(cartridge_t *cartridge, uint_fast16_t adr) 
{
    return cartridge->chr_memory[adr%cartridge->chr_size];
}

void cartridge_write_chr(   cartridge_t *cartridge, 
                            uint_fast16_t adr, 
                            uint_fast8_t value) 
{
    cartridge->chr_memory[adr%cartridge->chr_size]=value;
}
