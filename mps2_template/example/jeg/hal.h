#ifndef __JEG_HAL_H__
#define __JEG_HAL_H__

#include "app_platform.h"
#include "nes.h"
#include ".\common.h"
#include "jeg_cfg.h"

#define SCREEN_HEIGHT           240
#define SCREEN_WIDTH            256



typedef struct {
#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
    uint32_t wDirtyMatrix[SCREEN_HEIGHT >> 3];
#else
#   if JEG_DEBUG_SHOW_BACKGROUND == ENABLED
    compact_dual_pixels_t (*ptBuffer)[240][128];
#   endif

    union{
        uint8_t chBuffer[SCREEN_HEIGHT*(SCREEN_WIDTH>>1)];
        nes_screen_buffer_t chPixels;
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

#if JEG_DEBUG_SHOW_BACKGROUND == ENABLED
extern uint_fast8_t debug_fetch_color(uint_fast8_t chColor);
#endif

#endif
