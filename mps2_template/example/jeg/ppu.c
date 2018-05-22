#include "ppu.h"
#include "nes.h"

#include "common.h"
#include "jeg_cfg.h"

//! \name PPU Control Register bit mask
//! @{
#define PPUCTRL_NAMETABLE                   (3<<0)
#define PPUCTRL_INCREMENT                   (1<<2)
#define PPUCTRL_SPRITE_TABLE                (1<<3)
#define PPUCTRL_BACKGROUND_TABLE            (1<<4)
#define PPUCTRL_SPRITE_SIZE                 (1<<5)
#define PPUCTRL_MASTER_SLAVE                (1<<6)
#define PPUCTRL_NMI                         (1<<7)
//! @}

//! \name PPU Masking Register Bit Mask
//! @{
#define PPUMASK_GRAYSCALE                   (1<<0)
#define PPUMASK_SHOW_LEFT_BACKGROUND        (1<<1)
#define PPUMASK_SHOW_LEFT_SPRITES           (1<<2)
#define PPUMASK_SHOW_BACKGROUND             (1<<3)
#define PPUMASK_SHOW_SPRITES                (1<<4)
#define PPUMASK_RED_TINT                    (1<<5)
#define PPUMASK_GREEN_TINT                  (1<<6)
#define PPUMASK_BLUE_TINT                   (1<<7)
//! @}

//! \name PPU status register bit mask
//! @{
#define PPUSTATUS_SPRITE_OVERFLOW           (1<<5)
#define PPUSTATUS_SPRITE_ZERO_HIT           (1<<6)
#define PPUSTATUS_VBLANK                    (1<<7)
//! @}

#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
bool ppu_init(ppu_t *ppu, ppu_cfg_t *ptCFG) 
{
    bool bResult = false;
    do {
        if (NULL == ppu || NULL == ptCFG) {
            break;
        } else if (     (NULL == ptCFG->ptNES)
                    ||  (NULL == ptCFG->fnRead)
                    ||  (NULL == ptCFG->fnWrite)
                    ||  (NULL == ptCFG->fnDrawPixel)) {
            break;
        }
        
        ppu->nes                = ptCFG->ptNES;
        ppu->read               = ptCFG->fnRead;
        ppu->write              = ptCFG->fnWrite;
        ppu->fnDrawPixel        = ptCFG->fnDrawPixel;
        ppu->ptTag              = ptCFG->ptTag;
    #if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == DISABLED
        ppu->video_frame_data   = NULL;
    #endif
        ppu_reset(ppu);

        bResult = true;
    } while(false);
    return bResult;
}
#else
void ppu_init(ppu_t *ppu, nes_t *nes, ppu_read_func_t read, ppu_write_func_t write) 
{
    ppu->nes                = nes;
    ppu->read               = read;
    ppu->write              = write;
    ppu->video_frame_data   = NULL;
    ppu_reset(ppu);
}

void ppu_setup_video(ppu_t *ppu, uint8_t *video_frame_data) 
{
    ppu->video_frame_data = video_frame_data;

    memset(ppu->video_frame_data, 0, 256*240);
}
#endif


void ppu_reset(ppu_t *ppu) 
{
    ppu->last_cycle_number  = 0;
    ppu->cycle              = 340;
    ppu->scanline           = 240;
    ppu->ppuctrl            = 0;
    ppu->ppustatus          = 0;
    ppu->t                  = 0;

    memset(&(ppu->tNameAttributeTable[0]), 0, sizeof(name_attribute_table_t));
    memset(&(ppu->tNameAttributeTable[1]), 0, sizeof(name_attribute_table_t));
    
#if JEG_USE_4_PHYSICAL_NAME_ATTRIBUTE_TABLES == ENABLED
    memset(&(ppu->tNameAttributeTable[2]), 0, sizeof(name_attribute_table_t));
    memset(&(ppu->tNameAttributeTable[3]), 0, sizeof(name_attribute_table_t));
#endif

    ppu->ppumask            = 0;
    ppu->oam_address        = 0;
    ppu->register_data      = 0;
    ppu->name_table_byte    = 0;
    
#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == DISABLED
    if (NULL != ppu->video_frame_data) {
        memset(ppu->video_frame_data, 0, 256*240);
    }
#endif
}

uint_fast8_t ppu_read(ppu_t *ppu, uint_fast16_t hwAddress) 
{
    int value, buffered;

    switch (hwAddress & 0x07) {
        case 2:
            ppu_update(ppu);
            value=      (ppu->register_data&0x1F)
                    |   (ppu->ppustatus & (     PPUSTATUS_VBLANK
                                            |   PPUSTATUS_SPRITE_ZERO_HIT
                                            |   PPUSTATUS_SPRITE_OVERFLOW));
                                            
            ppu->ppustatus &= ~PPUSTATUS_VBLANK; // disable vblank flag
            ppu->w=0;
            break;
            
        case 4:
            value=ppu->tSpriteTable.chBuffer[ppu->oam_address];
            break;
            
        case 7:
            value=ppu->read(ppu->nes, ppu->v);
            if ((ppu->v & 0x3FFF) < 0x3F00) {
                buffered=ppu->buffered_data;
                ppu->buffered_data=value;
                value=buffered;
            } else {
                ppu->buffered_data=ppu->read(ppu->nes, ppu->v - 0x1000);
            }
            ppu->v+=((ppu->ppuctrl&PPUCTRL_INCREMENT)==0) ? 1 : 32;
            break;
            
        default:
            value=ppu->register_data;
            break;
    }
    return value;
}

void ppu_dma_access(ppu_t *ppu, uint_fast8_t chData)
{
    uint_fast16_t address_temp = chData << 8;

#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED

    uint8_t *pchSrc = ppu->nes->cpu.fnDMAGetSourceAddress(ppu->nes, address_temp);

    if (0 != ppu->oam_address) {
        uint_fast16_t hwSize = (256 - ppu->oam_address);

#   if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
        //! I am lazy, as this is the rare case, make it simple...
        ppu->bOAMUpdated = true;
#   endif
        
        memcpy(&(ppu->tSpriteTable.chBuffer[ppu->oam_address]), pchSrc, hwSize);
        pchSrc += hwSize;
        memcpy(&(ppu->tSpriteTable.chBuffer[0]), pchSrc, ppu->oam_address);
        
    } else {
#   if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
        uint8_t *pchCheck = pchSrc;
        uint8_t *pchOAM = ppu->tSpriteTable.chBuffer;
        uint_fast8_t n = 64;
        do {
            if (*pchCheck != *pchOAM) {
                ppu->bOAMUpdated = true;
                break;
            }
            pchCheck += 4;
            pchOAM += 4;
        } while(--n);
#   endif
        memcpy(&(ppu->tSpriteTable.chBuffer[0]), pchSrc, 256);
    }
#else
    for(uint_fast16_t i=0; i<256; i++) {
        int v=ppu->nes->cpu.read(ppu->nes, address_temp++);
#   if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
        if (!(i & 0x03)) {
            if (ppu->tSpriteTable.chBuffer[(ppu->oam_address + i) & 0xFF] != v) {
                ppu->bOAMUpdated = true;
            }
        }
#   endif
        ppu->tSpriteTable.chBuffer[(ppu->oam_address + i) & 0xFF]=v;
    }
#endif
    ppu->nes->cpu.stall_cycles += 513;
    if (ppu->nes->cpu.cycle_number & 0x01) {
        ppu->nes->cpu.stall_cycles++;
    }
}

void ppu_write(ppu_t *ppu, uint_fast16_t hwAddress, uint_fast8_t chData) 
{
    ppu->register_data = chData;

    switch (hwAddress & 7) {
        case 0:
            ppu->ppuctrl=chData;
            ppu->t = (ppu->t & 0xF3FF) | ((chData & 0x03) <<10 );
            break;
        case 1:
            ppu->ppumask=chData;
            break;
        case 3:
            ppu->oam_address=chData;
            break;
        case 4:
        #if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
            if (!(ppu->oam_address & 0x03)) {
                uint_fast8_t chOld = ppu->tSpriteTable.chBuffer[ppu->oam_address];
                if (chOld != chData) {
                    ppu->bOAMUpdated = true;
                }
            }
        #endif
            ppu->tSpriteTable.chBuffer[ppu->oam_address++] = chData;
            break;
        case 5:
            ppu_update(ppu);
            if (0 == ppu->w) {
                ppu->t = ( ppu->t & 0xFFE0 ) | ( chData>>3 );
                ppu->x = chData & 0x07;
                ppu->w = 1;
            } else {
                ppu->t = (ppu->t & 0x8FFF) | ((chData&0x07)<<12);
                ppu->t = (ppu->t & 0xFC1F) | ((chData&0xF8)<<2);
                ppu->w = 0;
            }
            break;
        case 6:
            if (0 == ppu->w) {
                ppu->t = (ppu->t&0x80FF) | ((chData&0x3F)<<8);
                ppu->w = 1;
            } else {
                ppu->t = (ppu->t&0xFF00) | chData;
                ppu->v = ppu->t;
                ppu->w = 0;
            }
            break;
        case 7:
            ppu->write(ppu->nes, ppu->v, chData);
            ppu->v += (0 == (ppu->ppuctrl & PPUCTRL_INCREMENT)) ? 1:32;
            break;
    }

}

static uint32_t fetch_sprite_pattern(ppu_t *ppu, sprite_t *ptSpriteInfo, uint_fast16_t hwRow) 
{
    uint_fast8_t tile = ptSpriteInfo->chIndex;
    uint_fast8_t chAttributes =  ptSpriteInfo->Attributes.chValue;
    uint_fast8_t table;
    uint_fast16_t hwAddress;

    if (ppu->ppuctrl & PPUCTRL_SPRITE_SIZE) {
        if ((chAttributes & 0x80)) {
            hwRow = 15 - hwRow;
        }
        table = tile & 0x01;
        tile &= 0xFE;
        
        if (hwRow > 7) {
            tile++;
            hwRow -= 8;
        }
    } else {
        if ((chAttributes & 0x80)) {
            hwRow = 7 - hwRow;
        }
        table = (ppu->ppuctrl & PPUCTRL_SPRITE_TABLE ? 1 : 0 ) ;
    }
    
    hwAddress = 0x1000 * table + tile * 16 + hwRow;
    
    uint_fast8_t low_tile_byte = ppu->read(ppu->nes, hwAddress);
    uint_fast8_t high_tile_byte = ppu->read(ppu->nes, hwAddress + 8);
    uint32_t data=0;
  
    uint_fast8_t p1, p2;
    if (chAttributes & 0x40) {
        uint_fast8_t n = 8;
        do {
            p1= (low_tile_byte & 0x01);
            p2= (high_tile_byte & 0x01) << 1;
            low_tile_byte >>= 1;
            high_tile_byte >>= 1;
            
            data <<= 4;
            data |= ((chAttributes & 3) << 2) | p1 | p2;
        } while(--n);
    } else {
        uint_fast8_t n = 8;
        do {
            p1 = (low_tile_byte & 0x80) >> 7;
            p2 = (high_tile_byte & 0x80) >> 6;
            low_tile_byte <<= 1;
            high_tile_byte <<= 1;

            data <<= 4;
            data |= ((chAttributes & 3) << 2) | p1 | p2;
        } while(--n);
    }

    return data;
}

#if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
static void sort_sprite_order_list(ppu_t *ptPPU)
{
    //! a very simple & stupid sorting algorithm 
    uint_fast8_t chIndex = 0, n;

    if (!ptPPU->bOAMUpdated) {
        return ;
    }
    ptPPU->bOAMUpdated = false;
    
    //! initialise the list
    for (;chIndex < 64; chIndex++) {
        ptPPU->SpriteYOrderList.List[chIndex].chIndex = chIndex;
        ptPPU->SpriteYOrderList.List[chIndex].chY = ptPPU->tSpriteTable.SpriteInfo[chIndex].chY;
    }
    
    //! sort the list
    for (chIndex = 0; chIndex < 64; chIndex++) {
        uint_fast16_t hwMin = 0xFFFF;
        uint_fast16_t hwMinIndex = chIndex;
        for (n = chIndex; n < 64; n++) {        
            if (ptPPU->SpriteYOrderList.List[n].chY < hwMin) {
                hwMinIndex = n;
                hwMin = ptPPU->SpriteYOrderList.List[n].chY;
            } 
        }
        
        if (hwMin >= 240) {
            //! no need to do it.
            break;
        } else if (chIndex == hwMinIndex) {
            continue;
        }  
        
        //! swap
        
        do {
            uint_fast8_t chTempIndex = ptPPU->SpriteYOrderList.List[hwMinIndex].chIndex;
            
            ptPPU->SpriteYOrderList.List[hwMinIndex].chIndex = ptPPU->SpriteYOrderList.List[chIndex].chIndex; 
            ptPPU->SpriteYOrderList.List[hwMinIndex].chY = ptPPU->SpriteYOrderList.List[chIndex].chY;
            
            ptPPU->SpriteYOrderList.List[chIndex].chIndex = chTempIndex;
            ptPPU->SpriteYOrderList.List[chIndex].chY = hwMin;
        } while(false);
    }
    
    ptPPU->SpriteYOrderList.chVisibleCount = chIndex;
    ptPPU->SpriteYOrderList.chCurrent = 0;
}
#endif

static inline uint_fast8_t fetch_sprite_info_on_specified_line(ppu_t *ptPPU, uint_fast32_t nScanLine)
{
    uint_fast8_t chCount = 0;
    uint_fast8_t chSpriteSize = ((ptPPU->ppuctrl & PPUCTRL_SPRITE_SIZE) ? 16 : 8);

#if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
    //! initialise sprite Y order sort list
    sort_sprite_order_list(ptPPU);

    for(int_fast32_t j = ptPPU->SpriteYOrderList.chCurrent; j < ptPPU->SpriteYOrderList.chVisibleCount; j++) {
        uint_fast8_t chIndex = ptPPU->SpriteYOrderList.List[j].chIndex;
        
        int_fast32_t row = nScanLine - ptPPU->SpriteYOrderList.List[j].chY;
        if (row < 0) {
            continue;
        } else if (row >= chSpriteSize) {
            //! don't check previous sprites
            ptPPU->SpriteYOrderList.chCurrent++;
            continue;
        }
        
        if (chCount < JEG_MAX_ALLOWED_SPRITES_ON_SINGLE_SCANLINE) {
            ptPPU->sprite_patterns[chCount]   = fetch_sprite_pattern(ptPPU, ptPPU->tSpriteTable.SpriteInfo + chIndex, row);
            ptPPU->sprite_positions[chCount]  = ptPPU->tSpriteTable.SpriteInfo[chIndex].chPosition; 
            ptPPU->sprite_priorities[chCount] = ptPPU->tSpriteTable.SpriteInfo[chIndex].Attributes.Priority;
            ptPPU->sprite_indicies[chCount]   = chIndex;
            chCount++;
        } else {
            break;
        }
    }
#else
    // evaluate sprite
    for(int_fast32_t j = 0; j < 64; j++) {
        int_fast32_t row = ptPPU->scanline-ptPPU->tSpriteTable.SpriteInfo[j].chY;
        if (    (row < 0)
            ||  (row >= chSpriteSize)) {
            continue;
        }
        if (chCount < JEG_MAX_ALLOWED_SPRITES_ON_SINGLE_SCANLINE) {
            ptPPU->sprite_patterns[chCount]   = fetch_sprite_pattern(ptPPU, ptPPU->tSpriteTable.SpriteInfo + j, row);
            ptPPU->sprite_positions[chCount]  = ptPPU->tSpriteTable.SpriteInfo[j].chPosition;
            ptPPU->sprite_priorities[chCount] = ptPPU->tSpriteTable.SpriteInfo[j].Attributes.Priority;
            ptPPU->sprite_indicies[chCount]   = j;
            chCount++;
        }
    }
#endif

    if (chCount > 8) {
        ptPPU->ppustatus |= PPUSTATUS_SPRITE_OVERFLOW;
    }
    return chCount;
}

static void fetch_background_tile_info(ppu_t *ptPPU)
{
    uint_fast32_t data = 0;
    ptPPU->tile_data <<= 4;
    
    uint_fast8_t chTableIndex = find_name_attribute_table_index(
                                        ptPPU->nes->cartridge.chMirror, 
                                        ptPPU->v & 0x0FFF);
    name_attribute_table_t *ptTable = &(ptPPU->tNameAttributeTable[chTableIndex]);
    
    uint_fast16_t hwAddress = ptPPU->v & 0x3FF;
    
    switch (ptPPU->cycle & 0x07) {
        case 1:                                                     //!< fetch name table byte
            ptPPU->name_table_byte 
                = ptTable->chBuffer[hwAddress];                     //!< ptPPU->read(ptPPU->nes, 0x2000 | (ptPPU->v&0x0FFF) );
            break;
            
        case 3:                                                     //!< fetch attribute table byte
            ptPPU->attribute_table_byte = ptTable->AttributeTable[ptPPU->tVAddress.YScroll>>2][ptPPU->tVAddress.XScroll>>2].chValue;
            ptPPU->attribute_table_byte = 
                  (   (   ptPPU->attribute_table_byte >> (    ( (ptPPU->v>>4) & 4) 
                                                          |   (  ptPPU->v&2)) 
                      ) & 3 
                  ) << 2;
            break;
            
        case 5:                                                     //!< fetch low tile byte
            ptPPU->low_tile_byte = ptPPU->read ( 
                        ptPPU->nes,    
                        0x1000*((ptPPU->ppuctrl & PPUCTRL_BACKGROUND_TABLE) ? 1 : 0)
                    +   ptPPU->name_table_byte*16
                    +   ptPPU->tVAddress.TileYOffsite
                );
            break;
            
        case 7:                                                     //!< fetch high tile byte
            ptPPU->high_tile_byte = ptPPU->read(
                        ptPPU->nes, 
                        0x1000 * ((ptPPU->ppuctrl & PPUCTRL_BACKGROUND_TABLE) ? 1 : 0)
                    +   ptPPU->name_table_byte*16
                    +   ptPPU->tVAddress.TileYOffsite + 8
                );
            break;
            
        case 0:                                                     //!< store tile data
            for(int_fast32_t j = 0; j<8; j++) {
                data <<= 4;
                data |=     ptPPU->attribute_table_byte
                        |   ((ptPPU->low_tile_byte  & 0x80) >> 7)
                        |   ((ptPPU->high_tile_byte & 0x80) >> 6);
                        
                ptPPU->low_tile_byte <<= 1;
                ptPPU->high_tile_byte <<= 1;
            }
            ptPPU->tile_data |= data;
            break;
    }
    


}

static void ppu_mix_background_and_foreground(ppu_t *ptPPU)
{
    //! render pixel
    uint_fast8_t background = 0, i = 0, sprite = 0;

    //! get sprite pixel color
    if (ptPPU->ppumask & PPUMASK_SHOW_SPRITES) {
    
        for(uint_fast8_t j = 0; j < ptPPU->sprite_count; j++) {
            int_fast16_t offset =   (ptPPU->cycle - 1) 
                                  - (int_fast16_t)ptPPU->sprite_positions[j];
                                  
            if ( offset < 0 || offset > 7) {
                continue;
            }
            
            int_fast32_t color = (ptPPU->sprite_patterns[j] >> ((7 - offset) * 4)) & 0x0F;
            if (!(color & 0x03)) {
                continue;
            }
            
            i = j;
            sprite = color;
            break;
        }
    }
    
    uint_fast8_t s = (sprite & 0x03), color = 0;

    //! get background pixel color
    if ((ptPPU->ppumask&PPUMASK_SHOW_BACKGROUND) != 0) {
        background = (ptPPU->tile_data >> (32 + ((7-ptPPU->x) * 4)) ) & 0x0F;
    }

    if ((ptPPU->cycle - 1) < 8) {
        if ((ptPPU->ppumask & PPUMASK_SHOW_LEFT_BACKGROUND) == 0) {
            background = 0;
        }
        if ((ptPPU->ppumask & PPUMASK_SHOW_LEFT_SPRITES) == 0) {
            sprite = 0;
        }
    }

    uint_fast8_t b = (background & 0x03);
    
    if (!b && s) {
        color = sprite | 0x10;
    } else if (b && !s) {
        color = background;
    } else if (b && s) {
        if (    (ptPPU->sprite_indicies[i] == 0) 
            &&  ((ptPPU->cycle - 1) < 255)) {
            ptPPU->ppustatus |= PPUSTATUS_SPRITE_ZERO_HIT;
        }
    
        if (ptPPU->sprite_priorities[i] == 0) {
            color = sprite | 0x10;
        } else {
            color = background;
        }
    }
    
    if ( color >= 16 && !(color & 0x03)) {
        color -= 16;
    }
#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
    ptPPU->fnDrawPixel(   ptPPU->ptTag, 
                        ptPPU->scanline,                              //!< Y
                        ptPPU->cycle-1,                               //!< X
                        ptPPU->palette[color]);                       //!< 8bit color
#else
    ptPPU->video_frame_data[ptPPU->scanline * 256 + ptPPU->cycle - 1] 
            = ptPPU->palette[color];
#endif
}


#define RENDERING_ENABLED       (ppu->ppumask & (   PPUMASK_SHOW_BACKGROUND     \
                                                |   PPUMASK_SHOW_SPRITES))
#define PRE_LINE                (261 == ppu->scanline)
#define VISIBLE_LINE            (ppu->scanline < 240)
#define RENDER_LINE             (PRE_LINE || VISIBLE_LINE)
#define PRE_FETCH_CYCLE         (ppu->cycle >= 321 && ppu->cycle <= 336)
#define VISIBLE_CYCLE           (ppu->cycle >= 1 && ppu->cycle <= 256)
#define FETCH_CYCLE             (PRE_FETCH_CYCLE || VISIBLE_CYCLE)

uint_fast32_t ppu_update(ppu_t *ppu) 
{
    //! tick
    int_fast32_t cycles = (ppu->nes->cpu.cycle_number - ppu->last_cycle_number) * 3;
    ppu->last_cycle_number = ppu->nes->cpu.cycle_number;

    while(cycles--) {
        ppu->cycle++;                                                           //!< go to next pixel
        
        if (ppu->cycle > 340) {                                                 //!< if scanline is rendered go to next scanline
            ppu->cycle = 0;
            ppu->scanline++;
            
            if (ppu->scanline > 261) {                                          //!< if frame is finished go to next frame
                ppu->scanline = 0;
                ppu->f^=1;
            }
        }

        //! render
        if (RENDERING_ENABLED) {
        
            //! background logic
            if (VISIBLE_LINE && VISIBLE_CYCLE) {
                ppu_mix_background_and_foreground(ppu);
            }
            
            if (RENDER_LINE && FETCH_CYCLE) {
                //! fetch background tile information with ppu->v 
                fetch_background_tile_info(ppu);
            }
            
            if (   PRE_LINE 
                && ppu->cycle >= 280 
                && ppu->cycle <= 304) {
                
                /* equivalent logic
                ppu->tVAddress.YToggleBit = ppu->tTempVAddress.YToggleBit;
                ppu->tVAddress.YScroll = ppu->tTempVAddress.YScroll;
                ppu->tVAddress.TileYOffsite = ppu->tTempVAddress.TileYOffsite;
                */
                ppu->v = (ppu->v & 0x841F) | (ppu->t & 0x7BE0);                 //!< ppu copy y
            }
            
            if (RENDER_LINE) {
                /*
                         (0,0)     (256,0)     (511,0)
                           +-----------+-----------+
                           |           |           |
                           |           |           |
                           |   $2000   |   $2400   |
                           |           |           |
                           |           |           |
                    (0,240)+-----------+-----------+(511,240)
                           |           |           |
                           |           |           |
                           |   $2800   |   $2C00   |
                           |           |           |
                           |           |           |
                           +-----------+-----------+
                         (0,479)   (256,479)   (511,479)
                         
                     The start location of the display window is determined 
                     by (X,Y) 
                        where X = (t.XScroll | t.XToggleBit) << 3 + ppu->x
                              Y = (t.YScroll | t.YToggleBit) << 3 + ppu->tVAddress.TileYOffsite
                */
            
                if (    FETCH_CYCLE 
                    &&  ((ppu->cycle & 0x07) == 0) ) {
                        
                    if (ppu->tVAddress.XScroll == 31) {
                        ppu->tVAddress.XToggleBit ^= 1;                         //! switch to another name table horizontally 
                    } 
                    ppu->tVAddress.XScroll++;
                }
                
                if (256 == ppu->cycle) {
                
                    if (ppu->tVAddress.TileYOffsite == 7) {
                        if (ppu->tVAddress.YScroll == 29) {
                            ppu->tVAddress.YToggleBit ^= 1;                     //! switch to another name table vertically 
                            ppu->tVAddress.YScroll = 0;
                        } else {
                            ppu->tVAddress.YScroll++;
                        }
                    }   
                    ppu->tVAddress.TileYOffsite++;

                } else if (ppu->cycle == 257) {
                    /* equivalent logic
                    ppu->tVAddress.XScroll = ppu->tTempVAddress.XScroll;
                    ppu->tVAddress.XToggleBit = ppu->tTempVAddress.XToggleBit;
                    */
                    ppu->v = (ppu->v & 0xFBE0) | (ppu->t & 0x41F);              //!< copy x
                }
            }

            // sprite logic
            if (257 == ppu->cycle) {
                if (VISIBLE_LINE) {
                    /*! fetch all the sprite informations on current scanline */
                    ppu->sprite_count = fetch_sprite_info_on_specified_line(ppu, ppu->scanline);
                    
                } else if (240 == ppu->scanline) {
                    //! reset sprite Y order list counter
                    ppu->SpriteYOrderList.chCurrent = 0;
                    
                } else {
                    ppu->sprite_count = 0;
                }
            }
        }

        if (241 == ppu->scanline && 1 == ppu->cycle) {
            ppu->ppustatus |= PPUSTATUS_VBLANK;
            
        #if JEG_USE_FRAME_SYNC_UP_FLAG  == ENABLED
            ppu->bFrameReady = true;
        #endif
            if (ppu->ppuctrl & PPUCTRL_NMI) {
                cpu6502_trigger_interrupt(&ppu->nes->cpu, INTERRUPT_NMI);
            }
        }

        if (    (260 == ppu->scanline) 
            &&  (329 == ppu->cycle)) {
            ppu->ppustatus &= ~(    PPUSTATUS_VBLANK
                                |   PPUSTATUS_SPRITE_ZERO_HIT
                                |   PPUSTATUS_SPRITE_OVERFLOW);
        }
    }

    return (341*262-((ppu->scanline+21)%262)*341-ppu->cycle)/3+1;
}

#if JEG_USE_FRAME_SYNC_UP_FLAG  == ENABLED
bool ppu_is_frame_ready(ppu_t *ptPPU)
{
    bool bResult = ptPPU->bFrameReady;
    ptPPU->bFrameReady = false;
    return bResult;
}
#endif
