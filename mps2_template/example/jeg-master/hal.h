#ifndef __JEG_HAL_H__
#define __JEG_HAL_H__

#include <stdint.h>
#include "app_platform.h"

#define SCREEN_HEIGHT           240
#define SCREEN_WIDTH            256


typedef struct {
    union{
        uint8_t chBuffer[SCREEN_HEIGHT*SCREEN_WIDTH];
        uint8_t chPixels[SCREEN_HEIGHT][SCREEN_WIDTH]
    };
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

#endif