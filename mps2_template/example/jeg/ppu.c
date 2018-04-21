#include "ppu.h"
#include "nes.h"
#include <stdio.h>
#include <string.h>

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

void ppu_init(ppu_t *ppu, nes_t *nes, ppu_read_func_t read, ppu_write_func_t write) {
  ppu->nes=nes;
  ppu->read=read;
  ppu->write=write;
  ppu->video_frame_data=0;
  ppu_reset(ppu);
}

void ppu_setup_video(ppu_t *ppu, uint8_t *video_frame_data) {
  ppu->video_frame_data=video_frame_data;
  for (int i=0; i<256*240; i++) {
    ppu->video_frame_data[i]=0;
  }
}

void ppu_reset(ppu_t *ppu) {
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

    if (NULL != ppu->video_frame_data) {
        memset(ppu->video_frame_data, 0, 256*240);
    }
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
            if (ppu->v%0x4000<0x3F00) {
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

    uint8_t *pchSrc = ppu->nes->cpu.fnDMAGetSourceAddress(ppu->nes, address_temp);
    memcpy(&(ppu->oam_data[ppu->oam_address]), pchSrc, 256);
    
    ppu->nes->cpu.stall_cycles += 513;
    if (ppu->nes->cpu.cycle_number & 0x01) {
        ppu->nes->cpu.stall_cycles++;
    }
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

inline uint32_t fetch_sprite_pattern(ppu_t *ppu, int i, int row) {
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
  for (int j=0; j<8; j++) {
    int p1, p2;
    if ((attributes&0x40)==0x40) {
      p1=(low_tile_byte&0x01);
      p2=(high_tile_byte&0x01)<<1;
      low_tile_byte>>=1;
      high_tile_byte>>=1;
    }
    else {
      p1=(low_tile_byte&0x80)>>7;
      p2=(high_tile_byte&0x80)>>6;
      low_tile_byte<<=1;
      high_tile_byte<<=1;
    }
    data<<=4;
    data|=((attributes&3)<<2)|p1|p2;
  }
  return data;
}

#define RENDERING_ENABLED (ppu->ppumask&(PPUMASK_SHOW_BACKGROUND|PPUMASK_SHOW_SPRITES))
#define PRE_LINE (ppu->scanline==261)
#define VISIBLE_LINE (ppu->scanline<240)
#define RENDER_LINE (PRE_LINE || VISIBLE_LINE)
#define PRE_FETCH_CYCLE (ppu->cycle>=321 && ppu->cycle<=336)
#define VISIBLE_CYCLE (ppu->cycle>=1 && ppu->cycle<=256)
#define FETCH_CYCLE (PRE_FETCH_CYCLE || VISIBLE_CYCLE)

int ppu_update(ppu_t *ppu) {
  // tick
  // TODO: every second frame is shorter

  while(ppu->nes->cpu.cycle_number>ppu->last_cycle_number) {
    for(int clock_div=0; clock_div<3; clock_div++) {
      ppu->cycle++; // go to next pixel
      if (ppu->cycle>340) { // if scanline is rendered go to next scanline
        ppu->cycle=0;
        ppu->scanline++;
        if (ppu->scanline>261) { // if frame is finished go to next frame
          ppu->scanline=0;
          ppu->f^=1;
        }
      }

      // render
      if (RENDERING_ENABLED) {
        // background logic

        if (VISIBLE_LINE && VISIBLE_CYCLE) {
          // render pixel
          int background=0, i=0, sprite=0;

          // get background pixel color
          if ((ppu->ppumask&PPUMASK_SHOW_BACKGROUND)!=0) {
            background=((ppu->tile_data>>32)>>((7-ppu->x)*4))&0x0F;
          }

          // get sprite pixel color
          if ((ppu->ppumask&PPUMASK_SHOW_SPRITES)!=0) {
            for(int j=0; j<ppu->sprite_count; j++) {
              int offset=(ppu->cycle-1)-(int)ppu->sprite_positions[j];
              if (offset<0 || offset>7) {
                continue;
              }
              int color=(ppu->sprite_patterns[j]>>((7-offset)*4))&0x0F;
              if (color%4==0) {
                continue;
              }
              i=j;
              sprite=color;
              break;
            }
          }

          if (ppu->cycle-1<8) {
            if ((ppu->ppumask&PPUMASK_SHOW_LEFT_BACKGROUND)==0) {
              background=0;
            }
            if ((ppu->ppumask&PPUMASK_SHOW_LEFT_SPRITES)==0) {
              sprite=0;
            }
          }

          int b=(background%4!=0), s=(sprite%4!=0), color=0;
          if (!b && s) {
            color=sprite|0x10;
          }
          else if (b && !s) {
            color=background;
          }
          else if (b && s) {
            if (ppu->sprite_indicies[i]==0 && ppu->cycle-1<255) {
              ppu->ppustatus|=PPUSTATUS_SPRITE_ZERO_HIT;
            }
            if (ppu->sprite_priorities[i]==0) {
              color=sprite|0x10;
            }
            else {
              color=background;
            }
          }
          if (color>=16 && color%4==0) {
            color-=16;
          }
          if (ppu->video_frame_data) {
            ppu->video_frame_data[ppu->scanline*256+ppu->cycle-1]=ppu->palette[color];
          }
        }

        if (RENDER_LINE && FETCH_CYCLE) {
          uint32_t data=0;
          ppu->tile_data<<=4;
          switch (ppu->cycle%8) {
            case 1: // fetch name table byte
              ppu->name_table_byte=ppu->read(ppu->nes, 0x2000|(ppu->v&0x0FFF));
              break;
            case 3: // fetch attribute table byte
              ppu->attribute_table_byte=((ppu->read(ppu->nes, 0x23C0|(ppu->v&0xC00)|((ppu->v>>4)&0x38)|((ppu->v>>2)&0x07))>>(((ppu->v>>4)&4)|(ppu->v&2)))&3)<<2;
              break;
            case 5: // fetch low tile byte
              ppu->low_tile_byte=ppu->read(ppu->nes, 0x1000*(ppu->ppuctrl&PPUCTRL_BACKGROUND_TABLE?1:0)+ppu->name_table_byte*16+((ppu->v>>12)&7));
              break;
            case 7: // fetch high tile byte
              ppu->high_tile_byte=ppu->read(ppu->nes, 0x1000*(ppu->ppuctrl&PPUCTRL_BACKGROUND_TABLE?1:0)+ppu->name_table_byte*16+((ppu->v>>12)&7)+8);
              break;
            case 0: // store tile data
              for(int j=0; j<8; j++) {
                data<<=4;
                data|=ppu->attribute_table_byte|((ppu->low_tile_byte&0x80)>>7)|((ppu->high_tile_byte&0x80)>>6);
                ppu->low_tile_byte<<=1;
                ppu->high_tile_byte<<=1;
              }
              ppu->tile_data|=data;
              break;
          }
        }
        if (PRE_LINE && ppu->cycle>=280 && ppu->cycle <= 304) {
          // ppu copy y
          ppu->v=(ppu->v&0x841F)|(ppu->t&0x7BE0);
        }
        if (RENDER_LINE) {
          if (FETCH_CYCLE && ppu->cycle%8==0) {
            // increment x
            if ((ppu->v&0x001F)==31) {
              ppu->v&=0xFFE0;
              ppu->v^=0x400;
            }
            else {
              ppu->v++;
            }
          }
          if (ppu->cycle==256) {
            // increment y
            if ((ppu->v&0x7000)!=0x7000) {
              ppu->v+=0x1000;
            }
            else {
              ppu->v&=0x8FFF;
              int y=(ppu->v&0x3E0)>>5;
              switch(y) {
                case 29:
                  ppu->v^=0x0800;
                case 31: // fallthrough from case 29
                  y=0;
                  break;
                default:
                  y++;
              }
              ppu->v=(ppu->v&0xFC1F)|(y<<5);
            }
          }
          else if (ppu->cycle==257) {
            // copy x
            ppu->v=(ppu->v&0xFBE0)|(ppu->t&0x41F);
          }
        }

        // sprite logic
        if (ppu->cycle==257) {
          if (VISIBLE_LINE) {
            // evaluate sprite
            int count=0;
            for(int j=0; j<64; j++) {
              int row=ppu->scanline-ppu->oam_data[j*4];
              if (row<0||row>=(ppu->ppuctrl&PPUCTRL_SPRITE_SIZE?16:8)) {
                continue;
              }
              if (count<8) {
                ppu->sprite_patterns[count]=fetch_sprite_pattern(ppu, j, row);
                ppu->sprite_positions[count]=ppu->oam_data[j*4+3];
                ppu->sprite_priorities[count]=(ppu->oam_data[j*4+2]>>5)&0x01;
                ppu->sprite_indicies[count]=j;
              }
              count++;
            }
            if (count>8) {
              count=8;
              ppu->ppustatus|=PPUSTATUS_SPRITE_OVERFLOW;
            }
            ppu->sprite_count=count;
          }
          else {
            ppu->sprite_count=0;
          }
        }
      }

      if (ppu->scanline==241 && ppu->cycle==1) {
        ppu->ppustatus|=PPUSTATUS_VBLANK;
        if (ppu->ppuctrl&PPUCTRL_NMI) {
          cpu6502_trigger_interrupt(&ppu->nes->cpu, INTERRUPT_NMI);
        }
      }

      if (ppu->scanline==260 && ppu->cycle==329) {
        ppu->ppustatus&=~(PPUSTATUS_VBLANK|PPUSTATUS_SPRITE_ZERO_HIT|PPUSTATUS_SPRITE_OVERFLOW);
      }
    }
    ppu->last_cycle_number++;
  }

  return (341*262-((ppu->scanline+21)%262)*341-ppu->cycle)/3+1;
}