/****************************************************************************
*  Copyright 2018 Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)       *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*     http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/

/*
IT License

Copyright (c) 2018 Günther Jena

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include ".\ppu_new.h"
//#include "nes.h"
#include <stdio.h>

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
  if (ppu->video_frame_data) {
    for (int i=0; i<256*240; i++) {
      ppu->video_frame_data[i]=0;
    }
  }
}

int ppu_read(ppu_t *ppu, int adr) {
  int value, buffered;

  switch (adr) {
    case 0x2002:
      ppu_update(ppu);
      value=(ppu->register_data&0x1F)|(ppu->ppustatus&(PPUSTATUS_VBLANK|PPUSTATUS_SPRITE_ZERO_HIT|PPUSTATUS_SPRITE_OVERFLOW));
      ppu->ppustatus&=~PPUSTATUS_VBLANK; // disable vblank flag
      ppu->w=0;
      break;
    case 0x2004:
      value=ppu->oam_data[ppu->oam_address];
      break;
    case 0x2007:
      value=ppu->read(ppu->nes, ppu->v);
      if (ppu->v%0x4000<0x3F00) {
        buffered=ppu->buffered_data;
        ppu->buffered_data=value;
        value=buffered;
      }
      else {
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

void ppu_write(ppu_t *ppu, int adr, int value) {
  int address_temp;

  ppu->register_data=value;

  switch (adr) {
    case 0x2000:
      ppu->ppuctrl=value;
      ppu->t=(ppu->t&0xF3FF)|((value&0x03)<<10);
      break;
    case 0x2001:
      ppu->ppumask=value;
      break;
    case 0x2003:
      ppu->oam_address=value;
      break;
    case 0x2004:
      ppu->oam_data[ppu->oam_address]=value;
      ppu->oam_address++;
      break;
    case 0x2005:
      ppu_update(ppu);
      if (ppu->w==0) {
        ppu->t=(ppu->t&0xFFE0)|(value>>3);
        ppu->x=value&0x07;
        ppu->w=1;
      }
      else {
        ppu->t=(ppu->t&0x8FFF)|((value&0x07)<<12);
        ppu->t=(ppu->t&0xFC1F)|((value&0xF8)<<2);
        ppu->w=0;
      }
      break;
    case 0x2006:
      if (ppu->w==0) {
        ppu->t=(ppu->t&0x80FF)|((value&0x3F)<<8);
        ppu->w=1;
      }
      else {
        ppu->t=(ppu->t&0xFF00)|value;
        ppu->v=ppu->t;
        ppu->w=0;
      }
      break;
    case 0x2007:
      ppu->write(ppu->nes, ppu->v, value);
      ppu->v+=((ppu->ppuctrl&PPUCTRL_INCREMENT)==0)?1:32;
      break;
    case 0x4014:
      address_temp=value<<8;
      for(int i=0; i<256; i++) {
        int v=ppu->nes->cpu.read(ppu->nes, address_temp);
        ppu->oam_data[ppu->oam_address+i]=v;
        address_temp++;
      }
      ppu->nes->cpu.stall_cycles+=513;
      if (ppu->nes->cpu.cycle_number%2) {
        ppu->nes->cpu.stall_cycles++;
      }
      break;
    default:
      // TODO: should not happen
      break;
  }
}

uint32_t fetch_sprite_pattern(ppu_t *ppu, int i, int row) {
  int tile=ppu->oam_data[i*4+1];
  int attributes=ppu->oam_data[i*4+2];
  uint16_t address;

  if ((ppu->ppuctrl&PPUCTRL_SPRITE_SIZE)==0) {
    if ((attributes&0x80)==0x80) {
      row=7-row;
    }
    address=0x1000*(ppu->ppuctrl&PPUCTRL_SPRITE_TABLE?1:0)+tile*16+row;
  }
  else {
    if ((attributes&0x80)==0x80) {
      row=15-row;
    }
    tile&=0xFE;
    if (row>7) {
      tile++;
      row-=8;
    }
    address=0x1000*(tile&0x01)+tile*16+row;
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
