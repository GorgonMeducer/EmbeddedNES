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
This file present all abstraction needed to port LiteNES.
  (The current working implementation uses allegro library.)

To port this project, replace the following functions by your own:
1) nes_hal_init()
    Do essential initialization work, including starting a FPS HZ timer.

2) nes_set_bg_color(c)
    Set the back ground color to be the NES internal color code c.

3) nes_flush_buf(*buf)
    Flush the entire pixel buf's data to frame buffer.

4) nes_flip_display()
    Fill the screen with previously set background color, and
    display all contents in the frame buffer.

5) wait_for_frame()
    Implement it to make the following code is executed FPS times a second:
        while (1) {
            wait_for_frame();
            do_something();
        }

6) int nes_key_state(int b) 
    Query button b's state (1 to be pressed, otherwise 0).
    The correspondence of b and the buttons:
      0 - Power
      1 - A
      2 - B
      3 - SELECT
      4 - START
      5 - UP
      6 - DOWN
      7 - LEFT
      8 - RIGHT
*/
#include "hal.h"
#include "fce.h"
#include "app_platform.h"


#define __USE_TILE__        (false)

/* Color coding (16-bit):
     15..11 = R4..0 (Red)
     10..5  = G5..0 (Green)
      4..0  = B4..0 (Blue)
*/
typedef union {
    struct {
        uint16_t    B   :5;
        uint16_t    G   :6;
        uint16_t    R   :5;
    };
    uint16_t hwValue;
}color_t;

typedef struct {
    color_t tColor;
}pixel_t;

#if __USE_TILE__
typedef pixel_t tile_t[8][8];

NO_INIT static tile_t s_tScreenBuffer[SCREEN_HEIGHT >> 3][SCREEN_WIDTH >> 3];
#else
NO_INIT static pixel_t s_tScreenBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
#endif

NO_INIT static color_t s_tColorMap[UBOUND(palette)];

/* Wait until next allegro timer event is fired. */
void wait_for_frame(void)
{
    //__nop();
}

/* Set background color. RGB value of c is defined in fce.h */
void nes_set_bg_color(int c)
{
    //GLCD_SetBackgroundColor(s_tColorMap[c].hwValue);
    //int32_t nValue = stop_counter();
    //start_counter();
    //log("Refresh Time: %d us\r     ", nValue / 25);
    
    uint32_t *pchBuffer = (uint32_t *)s_tScreenBuffer;
    uint32_t wCount = sizeof(s_tScreenBuffer)>>2;
    uint32_t wColor = s_tColorMap[c].hwValue | ((uint32_t)(s_tColorMap[c].hwValue) << 16);
    do {
        
        *pchBuffer++ = wColor;
        *pchBuffer++ = wColor;
        *pchBuffer++ = wColor;
        *pchBuffer++ = wColor;
        *pchBuffer++ = wColor;
        *pchBuffer++ = wColor;
        *pchBuffer++ = wColor;
        *pchBuffer++ = wColor;
        wCount -= 8;
    } while(wCount);
    
}

#if defined(USE_LITE_NES)
/* Flush the pixel buffer */
void nes_flush_buf(PixelBuf  * __restrict pbuff) {
    
    
    #if false
    uint32_t wCounter = pbuff->size;
    
    if (!wCounter) {
        return ;
    }
    
    Pixel *ptBuffer = pbuff->buf;
    Pixel *ptPix;
    switch(wCounter & 0x07) {
        do {
            case 0:
                wCounter--;
                ptPix = ptBuffer++;
                s_tScreenBuffer[ptPix->y][ptPix->x].tColor = s_tColorMap[ptPix->c];
            case 7:
                ptPix = ptBuffer++;
                s_tScreenBuffer[ptPix->y][ptPix->x].tColor = s_tColorMap[ptPix->c];
            case 6:
                ptPix = ptBuffer++;
                s_tScreenBuffer[ptPix->y][ptPix->x].tColor = s_tColorMap[ptPix->c];
            case 5:
                ptPix = ptBuffer++;
                s_tScreenBuffer[ptPix->y][ptPix->x].tColor = s_tColorMap[ptPix->c];
            case 4:
                ptPix = ptBuffer++;
                s_tScreenBuffer[ptPix->y][ptPix->x].tColor = s_tColorMap[ptPix->c];
            case 3:
                ptPix = ptBuffer++;
                s_tScreenBuffer[ptPix->y][ptPix->x].tColor = s_tColorMap[ptPix->c];
            case 2:
                ptPix = ptBuffer++;
                s_tScreenBuffer[ptPix->y][ptPix->x].tColor = s_tColorMap[ptPix->c];
            case 1:
                ptPix = ptBuffer++;
                s_tScreenBuffer[ptPix->y][ptPix->x].tColor = s_tColorMap[ptPix->c];
                wCounter &= ~0x07;
        } while(wCounter);
    }
    #else
    for (uint32_t i = 0; i < pbuff->size; i ++) {
        Pixel *p = &pbuff->buf[i];
        int x = p->x, y = p->y;
        /*if ((x >= SCREEN_WIDTH) || (y >= SCREEN_HEIGHT)) {
            __nop();
            continue;
        }*/
    #if __USE_TILE__
        //y = SCREEN_HEIGHT - y - 1;
        tile_t *ptTile = &s_tScreenBuffer[y>>3][x >> 3];
        (*ptTile)[7 - y & 0x07][x & 0x07].tColor = s_tColorMap[p->c];
        //s_tScreenBuffer[SCREEN_HEIGHT - y - 1][x].tColor = s_tColorMap[p->c];
    #else
        y = SCREEN_HEIGHT - y - 1;
        s_tScreenBuffer[y][x].tColor = s_tColorMap[p->c];
    #endif
    }
    #endif
    
}
#elif defined(USE_JEG)

extern void nes_flip_display(void);

void update_frame(frame_t *ptFrame) 
{
    int_fast32_t x = 0, y = 0;
    
    for (; y < SCREEN_HEIGHT; y++) {
        //int_fast32_t nTempY = SCREEN_HEIGHT - y - 1;
        for (x = 0; x < SCREEN_WIDTH; x++) {
            s_tScreenBuffer[y][x].tColor = s_tColorMap[ptFrame->chPixels[y][x]];
        }
    }
   
    nes_flip_display();
}
#endif

/* Initialization:
   (1) start a 1/FPS Hz timer. 
   (2) register fce_timer handle on each timer event */
void nes_hal_init(void)
{
    //! initialise LCD
    GLCD_Initialize();
    GLCD_SetBackgroundColor(GLCD_COLOR_BLACK);
    GLCD_ClearScreen();
    
    
    GLCD_FrameBufferAccess(true);
    
    
    do {
        //! initialise color map
        uint32_t n = 0;
        for (;n < UBOUND(s_tColorMap);n++) {
            pal_t color = palette[n];
            s_tColorMap[n].B = color.b >> 3;
            s_tColorMap[n].G = color.g >> 2;
            s_tColorMap[n].R = color.r >> 3;
        }
    } while(false);
}

/* Update screen at FPS rate by allegro's drawing function. 
   Timer ensures this function is called FPS times a second. */
void nes_flip_display(void)
{
    //static uint32_t s_wValue = 0;
    //s_wValue++;
    //if (!(s_wValue & 0x3)) {
    #if __USE_TILE__
    uint32_t x = 0, y =0;
    for (y = 16; y < (SCREEN_HEIGHT >> 3); y++) {
        for (x = 0; x < (SCREEN_WIDTH >> 3); x++) {
            GLCD_DrawBitmap(((320-SCREEN_WIDTH)>>1) + x * 8, y * 8, 8,8, (uint8_t *)&s_tScreenBuffer[y][x]);
        }
    }
    #else
        GLCD_DrawBitmap((320-SCREEN_WIDTH)>>1,0,SCREEN_WIDTH,SCREEN_HEIGHT, (uint8_t *)s_tScreenBuffer);
    #endif
    //}
}

/* Query a button's state.
   Returns 1 if button #b is pressed. */
uint_fast8_t nes_key_state(uint_fast8_t b)
{
    switch (b)
    {
        case 0: // On / Off
            return 1;
        /*
        case 1: // A
            return 0;al_key_down(&state, ALLEGRO_KEY_K);
        case 2: // B
            return al_key_down(&state, ALLEGRO_KEY_J);
        case 3: // SELECT
            return al_key_down(&state, ALLEGRO_KEY_U);
        case 4: // START
            return al_key_down(&state, ALLEGRO_KEY_I);
        case 5: // UP
            return al_key_down(&state, ALLEGRO_KEY_W);
        case 6: // DOWN
            return al_key_down(&state, ALLEGRO_KEY_S);
        case 7: // LEFT
            return al_key_down(&state, ALLEGRO_KEY_A);
        case 8: // RIGHT
            return al_key_down(&state, ALLEGRO_KEY_D);
        */
        default:
            return 0;
    }

    //return 0;
}

