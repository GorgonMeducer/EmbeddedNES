#ifndef __LITE_NES_HAL_H__
#define __LITE_NES_HAL_H__

#include "nes.h"
#include "app_platform.h"

struct Pixel {
    uint8_t x, y; // (x, y) coordinate
    uint8_t c; // RGB value of colors can be found in fce.h
};
typedef struct Pixel Pixel;

/* A buffer of pixels */
struct PixelBuf {
	Pixel buf[264 * 264];
	uint16_t size;
};
typedef struct PixelBuf PixelBuf;

extern PixelBuf bg, bbg, fg;

// clear a pixel buffer
#define pixbuf_clean(bf) \
	do { \
		(bf).size = 0; \
	} while (0)

// add a pending pixel into a buffer
#define pixbuf_add(bf, xa, ya, ca) \
	do { \
		if ((xa) < SCREEN_WIDTH && (ya) < SCREEN_HEIGHT) { \
			(bf).buf[(bf).size].x = (xa); \
			(bf).buf[(bf).size].y = (ya); \
			(bf).buf[(bf).size].c = (ca); \
			(bf).size++; \
        } \
	} while (0)

// fill the screen with background color
extern void nes_set_bg_color(int c);

// flush pixel buffer to frame buffer
extern void nes_flush_buf(PixelBuf *buf);

// display and empty the current frame buffer
extern void nes_flip_display(void);

// initialization
extern void nes_hal_init(void);

// query key-press status
extern uint_fast8_t nes_key_state(uint_fast8_t b);

#endif
