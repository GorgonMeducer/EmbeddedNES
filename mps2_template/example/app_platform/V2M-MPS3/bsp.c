/****************************************************************************
*  Copyright 2017 Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)       *
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

/*============================ INCLUDES ======================================*/
#include ".\app_cfg.h"


#include "Yamin.h"

#include "RTE_Components.h"             // Component selection
//#include "Board_LED.h"                  // ::Board Support:LED
//#include "Board_Touch.h"                // ::Board Support:Touchscreen
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // 

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static volatile bool s_bBlockingStyle = true;


void disable_blocking_style(void)
{
    s_bBlockingStyle = false;

}

void enable_blocking_style(void)
{
    s_bBlockingStyle = true;
}

bool is_blocking_style_enabled(void)
{
    return s_bBlockingStyle;
}

bool bsp_init(void)
{
    return true;
}

static volatile uint32_t s_wMsDownCounter = 0;
void bsp_1ms_event_handler(void)
{
    if (s_wMsDownCounter) {
        s_wMsDownCounter --;
    }
}

void delay_ms(uint32_t wMS)
{
    s_wMsDownCounter = wMS;
    while(s_wMsDownCounter);
}

typedef union {
    struct {
        uint32_t    B : 4;
        uint32_t    G : 4;
        uint32_t    R : 4;
        uint32_t      : 20;
    };
    uint32_t wValue;
}vga_pixel_t;

typedef struct {
    vga_pixel_t tPixels[128][512];
}vga_frame_t;

#define VGA_BUFFER          (*(vga_frame_t *)(0x41100000))

int32_t VGA_DrawBitmap (uint32_t x, 
                        uint32_t y, 
                        uint32_t width, 
                        uint32_t height, 
                        const uint8_t *bitmap)
{
    const uint32_t *pwPixels = (const uint32_t *)bitmap;
    if (y<128) {
        return 0;
    }
    y-= 128;
    do {
        memcpy(&VGA_BUFFER.tPixels[y++][x], pwPixels, width*4);
        pwPixels+= width;
        if (y >= 128) {
            break;
        }
    } while(--height);
    
    return 0;
}
  
/* EOF */
