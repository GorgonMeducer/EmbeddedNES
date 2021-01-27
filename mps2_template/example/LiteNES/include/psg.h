// #define psg_io_read(...) 0xFF
// #define psg_io_write(...) /**/

#include "common.h"

#ifndef PSG_H
#define PSG_H

extern unsigned char psg_joy1[8];

extern uint_fast8_t psg_io_read(uint_fast16_t address);
extern void psg_io_write(uint_fast16_t address, uint_fast8_t data);

#endif
