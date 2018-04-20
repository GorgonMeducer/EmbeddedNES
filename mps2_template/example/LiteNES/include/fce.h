#ifndef __FCE_H__
#define __FCE_H__

#include <stdint.h>

extern int32_t fce_load_rom(uint8_t *pchROM, uint_fast32_t wSize);
extern void fce_init(void);
extern void fce_run(void);
extern void fce_update_screen(void);



// Palette

typedef struct __pal {
	int r;
	int g;
	int b;
} pal_t;

extern const pal_t palette[64];


#endif
