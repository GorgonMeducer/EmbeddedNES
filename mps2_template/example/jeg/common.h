#ifndef __JEC_COMMON_H__
#define __JEC_COMMON_H__

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    nes_err_imcomplete_rom      = -7,
    nes_err_unsupported_mapper  = -6,
    nes_err_invalid_rom         = -5,
    nes_err_illegal_size        = -4,
    nes_err_illegal_parameter   = -3,
    nes_err_illegal_pointer     = -2,
    nes_err_unknown             = -1,
    nes_err_none                = 0,
    nes_ok                      = 0,
} nes_err_t;

#endif
