#ifndef __FCE_H__
#define __FCE_H__

#include ".\common.h"

extern int32_t fce_load_rom(uint8_t *pchROM, uint_fast32_t wSize);
extern void fce_init(void);
extern void fce_run(void);

#endif
