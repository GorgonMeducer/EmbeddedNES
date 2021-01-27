#include "nes.h"
#include "cpu6502_debug.h"
#include <string.h>
#include <stdbool.h>
#include "jeg_cfg.h"

extern 
void nes_draw_pixels(   void *ptTag, 
                        uint_fast8_t y, 
                        uint_fast8_t x, 
                        uint_fast8_t chColor);

uint_fast8_t cpu6502_bus_read (void *ref, uint_fast16_t address) 
{
    nes_t* nes=(nes_t *)ref;
    int value;

    if (address<0x2000) {
        return nes->ram_data[address & 0x7FF];
        
    } else if (address>=0x6000) {
        return cartridge_read_prg(&nes->cartridge, address);
        
    } else if (address<0x4000) {
        return ppu_read(&nes->ppu, address);
        
    } else if (address==0x4016) {
        value=nes->controller_shift_reg[0]&1;
        nes->controller_shift_reg[0]=(nes->controller_shift_reg[0]>>1)|0x80;
        return value;
        
    } else if (address==0x4017) {
        value=nes->controller_shift_reg[1]&1;
        nes->controller_shift_reg[1]=(nes->controller_shift_reg[1]>>1)|0x80;
        return value;
    }
    return 0;
}

void cpu6502_bus_write (void *ref, uint_fast16_t address, uint_fast8_t value) 
{
    nes_t* nes=(nes_t *)ref;

    if (address<0x2000) {
        nes->ram_data[address & 0x7FF] = value;
        
    } else if (address<0x4000) {
        ppu_write(&nes->ppu, address, value);
        
    } else if (address==0x4014) {
        ppu_dma_access(&nes->ppu, value);
        
    } else if (address==0x4016 && value&0x01) {
        nes->controller_shift_reg[0]=nes->controller_data[0];
        nes->controller_shift_reg[1]=nes->controller_data[1];
        
    } else if (address>=0x6000) {
        cartridge_write_prg(&nes->cartridge, address, value);
    } 
}

#if JEG_USE_EXTRA_16BIT_BUS_ACCESS == ENABLED
uint_fast16_t cpu6502_bus_readw (void *ref, uint_fast16_t hwAddress) 
{
    nes_t* nes=(nes_t *)ref;

    if ( hwAddress<0x2000 ) {
        return *(uint16_t *)&(nes->ram_data[hwAddress & 0x7FF]);
    } else  if (hwAddress>=0x6000) {
        return cartridge_readw_prg(&nes->cartridge, hwAddress);
    } else {
        return cpu6502_bus_read(ref, hwAddress) | (cpu6502_bus_read(ref, hwAddress + 1) << 8);
    }
}

void cpu6502_bus_writew (void *ref, uint_fast16_t hwAddress, uint_fast16_t hwValue) 
{
    // it is not used...
}
#endif

#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED
uint8_t *ppu_dma_get_source_address(void *ref, uint_fast16_t hwAddress)
{
    
    nes_t* nes=(nes_t *)ref;
    
    if ( hwAddress<0x2000 ) {
        return &(nes->ram_data[hwAddress & 0x7FF]);
    } else  if (hwAddress>=0x6000) {
        return cartridge_get_prg_src_address(&nes->cartridge, hwAddress);
    } else {
        return NULL;
    }

}
#endif


//! \brief name table mirroring look up table
const static uint_fast8_t mirror_lookup[20] = {
    0,0,1,1,                //!< vertical mirroring
    0,1,0,1,                //!< horizontal mirroring
    0,0,0,0,                //!< single screen mirroring 0
    1,1,1,1,                //!< single screen mirroring 1
    0,1,2,3                 //!< Full/No mirroring
};


uint_fast16_t mirror_address (uint_fast8_t chMode, uint_fast16_t hwAddress) 
{
    hwAddress = hwAddress & 0x0FFF;
    return mirror_lookup[chMode*4+(hwAddress>>10)]*0x400+(hwAddress&0x3ff);
}

uint_fast8_t find_name_attribute_table_index(uint_fast8_t chMode, uint_fast16_t hwAddress)
{
    return mirror_lookup[chMode*4+((hwAddress & 0x0FFF)>>10)];
}


uint_fast8_t ppu_bus_read (nes_t *ptNES, uint_fast16_t hwAddress) 
{
    uint_fast8_t chData;
    hwAddress &= 0x3FFF;
    
    if (hwAddress <0x2000) {
        chData=cartridge_read_chr(&ptNES->cartridge, hwAddress);
        
    } else if (hwAddress<0x3F00) {
        uint_fast8_t chPhysicTableIndex = 
            find_name_attribute_table_index(ptNES->cartridge.chMirror, hwAddress) ;
        chData = ptNES->ppu.tNameAttributeTable[chPhysicTableIndex].chBuffer[hwAddress & 0x3FF];
        
    } else if (hwAddress<0x4000) {
        hwAddress &= 0x1F;
        if (hwAddress>=16 && (!(hwAddress & 0x03))) {
            hwAddress-=16;
        }
        chData = ptNES->ppu.palette[hwAddress];
    }
    
    return chData;
}

static void write_name_attribute_table(nes_t *ptNES, uint_fast16_t hwAddress, uint_fast8_t chData)
{
    uint_fast8_t chPhysicTableIndex = 
        find_name_attribute_table_index(ptNES->cartridge.chMirror, hwAddress);
    name_attribute_table_t *ptTable = &(ptNES->ppu.tNameAttributeTable[chPhysicTableIndex]);
    
    hwAddress &= 0x3FF;
    
#if JEG_USE_BACKGROUND_BUFFERING == ENABLED
    uint8_t chOldData = ptTable->chBuffer[hwAddress];
#endif

    ptTable->chBuffer[hwAddress] = chData;
    
#if JEG_USE_BACKGROUND_BUFFERING == ENABLED
    if (chOldData == chData) {
        return ;
    }

    ptTable->bRequestRefresh = true;
    //! update dirty matrix
    do {
        if (hwAddress < 32 * 30) {
            uint_fast32_t wMask = _BV(hwAddress & 0x1F);

            //! update nametable dirty matrix directly
            ptTable->wDirtyMatrix[hwAddress>>5] |= wMask;
        
        } else {        
            //! addressing attribute table, update name table dirty matrix indirectly
            hwAddress -= 32 * 30;
            
/*! \note   Each byte controls the palette of a 32x32 pixel or 4x4 tile part of 
            the nametable and is divided into four 2-bit areas. Each area covers 
            16x16 pixels or 2x2 tiles, the size of a [?] block in Super Mario Bros. 
            Given palette numbers topleft, topright, bottomleft, bottomright, 
            each in the range 0 to 3, the value of the byte is
            
               2xx0    2xx1    2xx2    2xx3    2xx4    2xx5    2xx6    2xx7
             ,-------+-------+-------+-------+-------+-------+-------+-------.
             | 0 . 1 |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
        2xC0:| - + - | - + - | - + - | - + - | - + - | - + - | - + - | - + - |
             | 2 . 3 |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
             +-------+-------+-------+-------+-------+-------+-------+-------+
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
        2xC8:| - + - | - + - | - + - | - + - | - + - | - + - | - + - | - + - |
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
             +-------+-------+-------+-------+-------+-------+-------+-------+
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
        2xD0:| - + - | - + - | - + - | - + - | - + - | - + - | - + - | - + - |
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
             +-------+-------+-------+-------+-------+-------+-------+-------+
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
        2xD8:| - + - | - + - | - + - | - + - | - + - | - + - | - + - | - + - |
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
             +-------+-------+-------+-------+-------+-------+-------+-------+
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
        2xE0:| - + - | - + - | - + - | - + - | - + - | - + - | - + - | - + - |
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
             +-------+-------+-------+-------+-------+-------+-------+-------+
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
        2xE8:| - + - | - + - | - + - | - + - | - + - | - + - | - + - | - + - |
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
             +-------+-------+-------+-------+-------+-------+-------+-------+
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
        2xF0:| - + - | - + - | - + - | - + - | - + - | - + - | - + - | - + - |
             |   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
             +-------+-------+-------+-------+-------+-------+-------+-------+
        2xF8:|   .   |   .   |   .   |   .   |   .   |   .   |   .   |   .   |
             `-------+-------+-------+-------+-------+-------+-------+-------'
     
*/
             
            uint_fast8_t chTileY = (hwAddress >> 3) * 4;
            uint_fast8_t chTileX = (hwAddress & 0x07) * 4;
            
            const struct {
                uint_fast8_t chX;
                uint_fast8_t chY;
            } c_OffSite[] = {
                {0,0},
                {2,0},
                {0,2},
                {2,2}
            };
            
            for (uint_fast8_t chGroup = 0; chGroup < 4; chGroup++) {
                if ((chOldData & 0x03) != (chData & 0x03)) {
                    //! current group has been affected
                    
                    uint_fast8_t chY = chTileY + c_OffSite[chGroup].chY;
                    uint_fast8_t chX = chTileX + c_OffSite[chGroup].chX;
                    uint_fast32_t wMask = 0x03 << (chX);

                    ptTable->wDirtyMatrix[chY+1]    |= wMask;
                    ptTable->wDirtyMatrix[chY]      |= wMask;

                }
                chOldData >>= 2;
                chData >>= 2;
            }
        }
    
    } while(0);

#endif
}

void ppu_bus_write (nes_t *ptNES, uint_fast16_t hwAddress, uint_fast8_t chData) 
{
    hwAddress &= 0x3FFF;
    
    if (hwAddress<0x2000) {
        cartridge_write_chr(&ptNES->cartridge, hwAddress, chData);
        
    } else if (hwAddress<0x3F00) {
        write_name_attribute_table(ptNES, hwAddress, chData);
        
    } else if (hwAddress<0x4000) {
        hwAddress &= 0x1F;
        if (hwAddress>=16 && (!(hwAddress & 0x03))) {
            hwAddress-=16;
        }
        ptNES->ppu.palette[hwAddress] = chData;
    }
}

bool nes_init(nes_t *ptNES, nes_cfg_t *ptCFG) 
{ 
    bool bResult = false;

    do {
        
        if (    NULL == ptNES 
            ||  NULL == ptCFG) {
            break;
        } 
        ptNES->ptTarget = ptCFG->ptTarget;
        
        {
            cpu6502_cfg_t tCFG = {
                ptNES,
            };
            if (! cpu6502_init(&ptNES->cpu, &tCFG)) {
                break;
            } 
        }
        
        {
            ppu_cfg_t tCFG = {
                ptNES,
            };
            if (ppu_init(&ptNES->ppu, &tCFG)) {
                break;
            }
        }
        
        bResult = true;
        
    } while(false);
    
    return bResult;
}

nes_err_t nes_setup_rom(nes_t *ptNES, uint8_t *pchData, uint_fast32_t wSize) 
{
    nes_err_t tResult = nes_err_illegal_pointer;
    
    do {
        if (NULL == ptNES || NULL == pchData) {
            break;
        }

        ptNES->controller_data[0] = 0;
        ptNES->controller_data[1] = 0;
        ptNES->controller_shift_reg[0] = 0;
        ptNES->controller_shift_reg[1] = 0;

        tResult = cartridge_setup(&(ptNES->cartridge), pchData, wSize);
        if (nes_ok == tResult) {
            nes_reset(ptNES);
        }
    } while(0);
    
    return tResult;
}

#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == DISABLED
void nes_setup_video(nes_t *nes, uint8_t *video_frame_data) 
{
    ppu_setup_video(&nes->ppu, video_frame_data);
}
#endif

void nes_reset(nes_t *nes) 
{
    cpu6502_reset(&nes->cpu);
    ppu_reset(&nes->ppu);
    memset(&nes->ram_data, 0, 0x800);
}

void nes_iterate_frame(nes_t *nes) 
{
    cpu6502_run(&nes->cpu, ppu_update(&nes->ppu));
}

void nes_set_controller(nes_t *nes, uint8_t controller1, uint8_t controller2) 
{
    nes->controller_data[0] = controller1;
    nes->controller_data[1] = controller2;
}

int_fast32_t ppu_get_cpu_cycle_count(void *ptTarget)
{
    nes_t *ptNES = (nes_t *)ptTarget;
    return ptNES->cpu.cycle_number;
}

void ppu_update_cpu_stall_cycle(void *ptTarget, int_fast32_t nCycles)
{
    nes_t *ptNES = (nes_t *)ptTarget;
    ptNES->cpu.stall_cycles += nCycles;
}

uint_fast8_t ppu_get_cartridge_mirror_type(void *ptTarget)
{
    nes_t *ptNES = (nes_t *)ptTarget;
    return ptNES->cartridge.chMirror;
}

 
void ppu_trigger_cpu_nmi(void *ptTarget)
{
    nes_t *ptNES = (nes_t *)ptTarget;
    cpu6502_trigger_interrupt(&(ptNES->cpu), INTERRUPT_NMI);
}



void ppu_draw_pixel(void *ptTarget, 
                    uint_fast8_t chY, 
                    uint_fast8_t chX, 
                    uint_fast8_t chColor)
{
    nes_t *ptNES = (nes_t *)ptTarget;
    nes_draw_pixels(ptNES->ptTarget, chY, chX, chColor);
}

#if JEG_USE_FRAME_SYNC_UP_FLAG  == ENABLED
bool nes_is_frame_ready(nes_t *ptNES)
{
    return ppu_is_frame_ready(&(ptNES->ppu));
}
#endif
