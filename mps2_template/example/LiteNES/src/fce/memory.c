#include "memory.h"
#include "cpu.h"
#include "ppu.h"
#include "psg.h"



uint_fast8_t memory_readb(uint_fast16_t address)
{
    uint_fast16_t hwTempAddress = address >> 13;

    if          (hwTempAddress > 3) {
        return mmc_read(address);
    } else if   (3 == hwTempAddress) {
        return cpu_ram_read(address & 0x1FFF);
    } else if   (0 == hwTempAddress) {
        return cpu_ram_read(address & 0x07FF);
    } else if  (1 == hwTempAddress) {
        return ppu_io_read(address);
    } else {
        return psg_io_read(address); 
    }

}

#if true
void memory_writeb(uint_fast16_t address, uint_fast8_t data)
{
    if (address == 0x4014) {
        byte *pchAddress = get_cpu_ram_address(0x100 * data);
    
        ppu_sprram_write_block(pchAddress, 256);
        return;
    }
    
    uint_fast16_t hwAddress = address >> 13;
    

    switch (hwAddress) {
        case 0: {cpu_ram_write(address & 0x07FF, data);return; }
        case 1: {ppu_io_write(address, data); return;}
        case 2: {psg_io_write(address, data); return;}
        case 3: {cpu_ram_write(address & 0x1FFF, data);return; }
        default: mmc_write(address, data);return;
    }  

}
#endif

uint_fast16_t memory_readw(uint_fast16_t address)
{
    uint_fast16_t hwTempAddress = address >> 13;

    if          (0 == hwTempAddress) {
        return cpu_ram_readw(address & 0x07FF);
    } else if   (hwTempAddress > 3) {
        return mmc_readw(address);
    } else if   (3 == hwTempAddress) {
        return cpu_ram_readw(address & 0x1FFF);
    } else  if   (1 == hwTempAddress) {
        return ppu_io_read(address) + (ppu_io_read(address + 1) << 8);
    } else {
        return psg_io_read(address) + (psg_io_read(address + 1) << 8); 
    }
}

void memory_writew(uint_fast16_t address, uint_fast16_t data)
{

    do {
        if (address == 0x4014) {
            break;
        }
        
        switch (address >> 13) {
            case 0: 
                cpu_ram_writew(address & 0x07FF, data);
                return; 
            case 1: //{ppu_io_write(address, data); return;}
            case 2: //{psg_io_write(address, data); return;}
                goto mww_use_default;
                //break;
            case 3: 
                cpu_ram_writew(address & 0x1FFF, data);
                return;
            default: 
                mmc_writew(address, data);
                return;
        }
        
        //return ;
    } while(false);
    
mww_use_default:

    memory_writeb(address, data & 0xFF);
    memory_writeb(address + 1, data >> 8);
}
