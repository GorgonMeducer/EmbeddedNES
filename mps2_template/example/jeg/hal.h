#ifndef __JEG_HAL_H__
#define __JEG_HAL_H__

#include <stdint.h>
#include "app_platform.h"
#include "jeg_cfg.h"

#define SCREEN_HEIGHT           240
#define SCREEN_WIDTH            256


typedef struct {
#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
    uint32_t wDirtyMatrix[SCREEN_HEIGHT >> 3];
#else
    union{
        uint8_t chBuffer[SCREEN_HEIGHT*SCREEN_WIDTH];
        uint8_t chPixels[SCREEN_HEIGHT][SCREEN_WIDTH];
    };
#endif
    
    uint16_t hwWidth;
    uint16_t hwHeight;
} frame_t;

typedef struct __pal {
	int r;
	int g;
	int b;
} pal_t;

extern const pal_t palette[64];

extern void nes_hal_init(void);
extern void update_frame(frame_t *ptFrame);

#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
extern void draw_pixels(void *ptTag, uint_fast8_t y, uint_fast8_t x, uint_fast8_t chColor);
#endif

#endif