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

#ifndef PPU_H
#define PPU_H

#include <stdint.h>

typedef struct nes_t nes_t;

typedef int (*ppu_read_func_t) (nes_t *nes, int address); // read data [8bit] from address [16bit]
typedef void (*ppu_write_func_t) (nes_t *nes, int address, int value); // write data [8bit] to address [16bit]
typedef void (*ppu_update_frame_func_t) (void *reference, uint8_t* frame_data, int width, int height);

typedef struct ppu_t {
  nes_t *nes; // reference to nes console

  // ppu state
  uint64_t last_cycle_number; // measured in cpu cycles
  int cycle;
  int scanline;

  int palette[32];
  int name_table[2048];
  int oam_data[256];

  // ppu registers
  int v; // current vram address (15bit)
  int t; // temporary vram address (15bit)
  int x; // fine x scoll (3bit)
  int w; // toggle bit (1bit)
  int f; // even/odd frame flag (1bit)

  int register_data;

  // background temporary variables
  int name_table_byte;
  int attribute_table_byte;
  int low_tile_byte;
  int high_tile_byte;
  uint64_t tile_data;

  // sprite temporary variables
  int sprite_count;
  uint32_t sprite_patterns[8];
  int sprite_positions[8];
  int sprite_priorities[8];
  int sprite_indicies[8];

  // memory accessable registers
  int ppuctrl;
  int ppumask;
  int ppustatus;
  int oam_address;
  int buffered_data;
   
  // memory interface to vram and vrom
  ppu_read_func_t read;
  ppu_write_func_t write;

  // frame data interface
  uint8_t *video_frame_data;
} ppu_t;

void ppu_init(ppu_t *ppu, nes_t *nes, ppu_read_func_t read, ppu_write_func_t write);
void ppu_setup_video(ppu_t *ppu, uint8_t *video_frame_data);

void ppu_reset(ppu_t *ppu);

int ppu_read(ppu_t *ppu, int adr); // read data [8bit] from address [16bit]
void ppu_write(ppu_t *ppu, int adr, int value); // write data [8bit] to address [16bit]

int ppu_update(ppu_t *ppu); // update ppu to current cpu cycle, return number of cpu cycles to next frame

#endif
