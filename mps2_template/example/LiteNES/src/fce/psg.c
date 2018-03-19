#include "psg.h"
#include "hal.h"

static byte prev_write;
static int p = 10;

inline uint_fast8_t psg_io_read(uint_fast16_t address)
{
    // Joystick 1
    if (address == 0x4016) {
        if (p++ < 9) {
            return nes_key_state(p);
        }
    }
    return 0;
}

inline void psg_io_write(uint_fast16_t address, uint_fast8_t data)
{
    if (address == 0x4016) {
        if ((data & 1) == 0 && prev_write == 1) {
            // strobe
            p = 0;
        }
    }
    prev_write = data & 1;
}
