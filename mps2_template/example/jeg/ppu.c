#include "ppu.h"
#include "nes.h"
#include <stdio.h>
#include <string.h>
#include "jeg_cfg.h"

#define PPUCTRL_NAMETABLE 3
#define PPUCTRL_INCREMENT 4
#define PPUCTRL_SPRITE_TABLE 8
#define PPUCTRL_BACKGROUND_TABLE 16
#define PPUCTRL_SPRITE_SIZE 32
#define PPUCTRL_MASTER_SLAVE 64
#define PPUCTRL_NMI 128

#define PPUMASK_GRAYSCALE 1
#define PPUMASK_SHOW_LEFT_BACKGROUND 2
#define PPUMASK_SHOW_LEFT_SPRITES 4
#define PPUMASK_SHOW_BACKGROUND 8
#define PPUMASK_SHOW_SPRITES 16
#define PPUMASK_RED_TINT 32
#define PPUMASK_GREEN_TINT 64
#define PPUMASK_BLUE_TINT 128

#define PPUSTATUS_SPRITE_OVERFLOW 32
#define PPUSTATUS_SPRITE_ZERO_HIT 64
#define PPUSTATUS_VBLANK 128

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
        
        ppu->nes=ptCFG->ptNES;
        ppu->read=ptCFG->fnRead;
        ppu->write=ptCFG->fnWrite;
        ppu->fnDrawPixel = ptCFG->fnDrawPixel;
        ppu->ptTag = ptCFG->ptTag;
    #if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == DISABLED
        ppu->video_frame_data = NULL;
    #endif
        ppu_reset(ppu);
            
        
        bResult = true;
    } while(false);
    return bResult;
}
#else
void ppu_init(ppu_t *ppu, nes_t *nes, ppu_read_func_t read, ppu_write_func_t write) 
{
    ppu->nes=nes;
    ppu->read=read;
    ppu->write=write;
    ppu->video_frame_data = NULL;
    ppu_reset(ppu);
}

void ppu_setup_video(ppu_t *ppu, uint8_t *video_frame_data) 
{
    ppu->video_frame_data=video_frame_data;

    memset(ppu->video_frame_data, 0, 256*240);
}
#endif


void ppu_reset(ppu_t *ppu) 
{
    ppu->last_cycle_number=0;
    ppu->cycle=340;
    ppu->scanline=240;
    ppu->ppuctrl=0;
    ppu->ppustatus=0;
    ppu->t=0;
    ppu->ppumask=0;
    ppu->oam_address=0;
    ppu->register_data=0;
    ppu->name_table_byte=0;
    
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
            value=ppu->oam_data[ppu->oam_address];
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
            ppu->v+=((ppu->ppuctrl&PPUCTRL_INCREMENT)==0)?1:32;
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
    memcpy(&(ppu->oam_data[ppu->oam_address]), pchSrc, 256);
#else
    for(uint_fast16_t i=0; i<256; i++) {
        int v=ppu->nes->cpu.read(ppu->nes, address_temp++);
        ppu->oam_data[ppu->oam_address+i]=v;
    }
#endif
    ppu->nes->cpu.stall_cycles += 513;
    if (ppu->nes->cpu.cycle_number & 0x01) {
        ppu->nes->cpu.stall_cycles++;
    }
    ppu->bOAMUpdated = true;
}

void ppu_write(ppu_t *ppu, uint_fast16_t hwAddress, uint_fast8_t chData) 
{
    int address_temp;

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
            ppu->oam_data[ppu->oam_address] = chData;
            ppu->oam_address++;
            ppu->bOAMUpdated = true;
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

uint32_t fetch_sprite_pattern(ppu_t *ppu, int i, int row) 
{
    int tile=ppu->oam_data[i*4+1];
    int attributes=ppu->oam_data[i*4+2];
    int table;
    uint16_t address;

    if ((ppu->ppuctrl&PPUCTRL_SPRITE_SIZE)==0) {
        if ((attributes&0x80)) {
            row=7-row;
        }
        address=0x1000*(ppu->ppuctrl&PPUCTRL_SPRITE_TABLE?1:0)+tile*16+row;
    } else {
        if ((attributes&0x80)) {
            row=15-row;
        }
        table=tile&0x01;
        tile&=0xFE;
        
        if (row>7) {
            tile++;
            row-=8;
        }
        address = 0x1000*(table)+tile*16+row;
    }
    
    int low_tile_byte=ppu->read(ppu->nes, address);
    int high_tile_byte=ppu->read(ppu->nes, address+8);
    uint32_t data=0;
  
    int p1, p2;
    if (attributes&0x40) {
        for (int j=0; j<8; j++) {
            p1=(low_tile_byte&0x01);
            p2=(high_tile_byte&0x01)<<1;
            low_tile_byte>>=1;
            high_tile_byte>>=1;
            
            data<<=4;
            data|=((attributes&3)<<2)|p1|p2;
        }
    } else {
        for (int j=0; j<8; j++) {
            p1=(low_tile_byte&0x80)>>7;
            p2=(high_tile_byte&0x80)>>6;
            low_tile_byte<<=1;
            high_tile_byte<<=1;

            data<<=4;
            data|=((attributes&3)<<2)|p1|p2;
        }
    }

    return data;
}


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
        ptPPU->SpriteYOrderList.List[chIndex].chY = ptPPU->SpriteInfo[chIndex].chY;
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

static inline uint_fast8_t fetch_sprite_info_on_specified_line(ppu_t *ptPPU, uint_fast32_t nScanLine)
{
    uint_fast8_t chCount = 0;
    uint_fast8_t chSpriteSize = ((ptPPU->ppuctrl & PPUCTRL_SPRITE_SIZE) ? 16 : 8);

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
            ptPPU->sprite_patterns[chCount]   = fetch_sprite_pattern(ptPPU, chIndex, row);
            ptPPU->sprite_positions[chCount]  = ptPPU->SpriteInfo[chIndex].chPosition; 
            ptPPU->sprite_priorities[chCount] = ptPPU->SpriteInfo[chIndex].Attributes.Priority;
            ptPPU->sprite_indicies[chCount]   = chIndex;
            chCount++;
        } else {
            break;
        }
    }

    if (chCount > 8) {
        ptPPU->ppustatus |= PPUSTATUS_SPRITE_OVERFLOW;
    }
    return chCount;
}

#define RENDERING_ENABLED       (ppu->ppumask & (   PPUMASK_SHOW_BACKGROUND     \
                                                |   PPUMASK_SHOW_SPRITES))
#define PRE_LINE                (261 == ppu->scanline)
#define VISIBLE_LINE            (ppu->scanline < 240)
#define RENDER_LINE             (PRE_LINE || VISIBLE_LINE)
#define PRE_FETCH_CYCLE         (ppu->cycle >= 321 && ppu->cycle <= 336)
#define VISIBLE_CYCLE           (ppu->cycle >= 1 && ppu->cycle <= 256)
#define FETCH_CYCLE             (PRE_FETCH_CYCLE || VISIBLE_CYCLE)

int_fast32_t ppu_update(ppu_t *ppu) 
{
    //! tick
    //! TODO: every second frame is shorter
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
                //! render pixel
                int_fast32_t background = 0, i = 0, sprite = 0;

                //! get background pixel color
                if ((ppu->ppumask&PPUMASK_SHOW_BACKGROUND) != 0) {
                    background = ((ppu->tile_data >> 32) >> ((7-ppu->x) * 4)) & 0x0F;
                }

                //! get sprite pixel color
                if ((ppu->ppumask&PPUMASK_SHOW_SPRITES)!=0) {
                
                    for(int_fast32_t j = 0; j < ppu->sprite_count; j++) {
                        int_fast32_t offset =   (ppu->cycle - 1) 
                                              - (int_fast32_t)ppu->sprite_positions[j];
                                              
                        if ( offset < 0 || offset > 7) {
                            continue;
                        }
                        
                        int_fast32_t color = (ppu->sprite_patterns[j] >> ((7 - offset) * 4)) & 0x0F;
                        if (color % 4 == 0) {
                            continue;
                        }
                        
                        i=j;
                        sprite = color;
                        break;
                    }
                }

                if ((ppu->cycle - 1) < 8) {
                    if ((ppu->ppumask & PPUMASK_SHOW_LEFT_BACKGROUND) == 0) {
                        background=0;
                    }
                    if ((ppu->ppumask & PPUMASK_SHOW_LEFT_SPRITES) == 0) {
                        sprite=0;
                    }
                }

                int_fast32_t b = (background % 4 !=0 ), s = (sprite % 4 !=0 ), color = 0;
                
                if (!b && s) {
                    color = sprite | 0x10;
                } else if (b && !s) {
                    color = background;
                } else if (b && s) {
                    if (    (ppu->sprite_indicies[i] == 0) 
                        &&  ((ppu->cycle - 1) < 255)) {
                        ppu->ppustatus|=PPUSTATUS_SPRITE_ZERO_HIT;
                    }
                
                    if (ppu->sprite_priorities[i] == 0) {
                        color=sprite|0x10;
                    } else {
                        color=background;
                    }
                }
                
                if ( color >= 16 && color % 4 == 0 ) {
                    color -= 16;
                }
            #if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
                ppu->fnDrawPixel(   ppu->ptTag, 
                                    ppu->scanline,                              //!< Y
                                    ppu->cycle-1,                               //!< X
                                    ppu->palette[color]);                       //!< 8bit color
            #else
                ppu->video_frame_data[ppu->scanline * 256 + ppu->cycle - 1] 
                        = ppu->palette[color];
            #endif
            }

            if (RENDER_LINE && FETCH_CYCLE) {
                uint_fast32_t data = 0;
                ppu->tile_data <<= 4;
                switch (ppu->cycle & 0x07) {
                    case 1:                                                     //!< fetch name table byte
                        ppu->name_table_byte 
                            = ppu->read(ppu->nes, 0x2000 | (ppu->v&0x0FFF) );
                        break;
                        
                    case 3:                                                     //!< fetch attribute table byte
                        ppu->attribute_table_byte
                            = (   (   ppu->read( ppu->nes,  (   0x23C0
                                                            |   ( ppu->v & 0xC00)
                                                            |   ((ppu->v >> 4) & 0x38)
                                                            |   ((ppu->v >> 2) & 0x07))
                                                            
                                               ) >> (       ( (ppu->v>>4) & 4) 
                                                        |   (ppu->v&2)) 
                                  ) & 3 
                              ) << 2;
                        break;
                        
                    case 5:                                                     //!< fetch low tile byte
                        ppu->low_tile_byte = ppu->read ( 
                                    ppu->nes,    
                                    0x1000*((ppu->ppuctrl & PPUCTRL_BACKGROUND_TABLE) ? 1 : 0)
                                +   ppu->name_table_byte*16
                                +   ((ppu->v>>12)&7)
                            );
                        break;
                        
                    case 7:                                                     //!< fetch high tile byte
                        ppu->high_tile_byte = ppu->read(
                                    ppu->nes, 
                                    0x1000 * ((ppu->ppuctrl & PPUCTRL_BACKGROUND_TABLE) ? 1 : 0)
                                +   ppu->name_table_byte*16
                                +   ((ppu->v >> 12) & 7) + 8
                            );
                        break;
                        
                    case 0:                                                     //!< store tile data
                        for(int_fast32_t j = 0; j<8; j++) {
                            data <<= 4;
                            data |=     ppu->attribute_table_byte
                                    |   ((ppu->low_tile_byte  & 0x80) >> 7)
                                    |   ((ppu->high_tile_byte & 0x80) >> 6);
                                    
                            ppu->low_tile_byte <<= 1;
                            ppu->high_tile_byte <<= 1;
                        }
                        ppu->tile_data |= data;
                        break;
                }
            }
            
            if (   PRE_LINE 
                && ppu->cycle >= 280 
                && ppu->cycle <= 304) {
                
                ppu->v = (ppu->v & 0x841F) | (ppu->t & 0x7BE0);                 //!< ppu copy y
            }
            
            if (RENDER_LINE) {
                if (    FETCH_CYCLE 
                    &&  ((ppu->cycle & 0x07) == 0) ) {
                    //! increment x
                    if ((ppu->v & 0x001F) == 31) {                              //!< wraps from 31 to 0, bit 10 is switched
                        ppu->v &= 0xFFE0;
                        ppu->v ^= _BV(10);
                    } else {
                        ppu->v++;
                    }
                }
                
                if (256 == ppu->cycle) {
                
                    //! increment y
                    if ((ppu->v & 0x7000) != 0x7000) {
                        ppu->v += 0x1000;
                    } else {
                        ppu->v &= 0x8FFF;
                        int_fast32_t y = (ppu->v & 0x3E0)>>5;
                        
                        switch(y) {
                            case 29:
                                ppu->v ^= _BV(11);
                            case 31:                                            //!< fallthrough from case 29
                                y = 0;
                                break;
                            default:
                                y++;
                        }
                        ppu->v = (ppu->v & 0xFC1F) | (y<<5);
                    }
                } else if (ppu->cycle == 257) {
                    
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

