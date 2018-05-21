#include "nes.h"
#include "cpu6502_debug.h"

#include "hal.h"
#include ".\common.h"
#include "jeg_cfg.h"

#ifndef this
#   define this        (*ptThis)
#endif

#define NES_PROFILINE       DISABLED 

const pal_t palette[64] = {
	{ 0x80, 0x80, 0x80 },
	{ 0x00, 0x00, 0xBB },
	{ 0x37, 0x00, 0xBF },
	{ 0x84, 0x00, 0xA6 },
	{ 0xBB, 0x00, 0x6A },
	{ 0xB7, 0x00, 0x1E },
	{ 0xB3, 0x00, 0x00 },
	{ 0x91, 0x26, 0x00 },
	{ 0x7B, 0x2B, 0x00 },
	{ 0x00, 0x3E, 0x00 },
	{ 0x00, 0x48, 0x0D },
	{ 0x00, 0x3C, 0x22 },
	{ 0x00, 0x2F, 0x66 },
	{ 0x00, 0x00, 0x00 },
	{ 0x05, 0x05, 0x05 },
	{ 0x05, 0x05, 0x05 },
	{ 0xC8, 0xC8, 0xC8 },
	{ 0x00, 0x59, 0xFF },
	{ 0x44, 0x3C, 0xFF },
	{ 0xB7, 0x33, 0xCC },
	{ 0xFF, 0x33, 0xAA },
	{ 0xFF, 0x37, 0x5E },
	{ 0xFF, 0x37, 0x1A },
	{ 0xD5, 0x4B, 0x00 },
	{ 0xC4, 0x62, 0x00 },
	{ 0x3C, 0x7B, 0x00 },
	{ 0x1E, 0x84, 0x15 },
	{ 0x00, 0x95, 0x66 },
	{ 0x00, 0x84, 0xC4 },
	{ 0x11, 0x11, 0x11 },
	{ 0x09, 0x09, 0x09 },
	{ 0x09, 0x09, 0x09 },
	{ 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x95, 0xFF },
	{ 0x6F, 0x84, 0xFF },
	{ 0xD5, 0x6F, 0xFF },
	{ 0xFF, 0x77, 0xCC },
	{ 0xFF, 0x6F, 0x99 },
	{ 0xFF, 0x7B, 0x59 },
	{ 0xFF, 0x91, 0x5F },
	{ 0xFF, 0xA2, 0x33 },
	{ 0xA6, 0xBF, 0x00 },
	{ 0x51, 0xD9, 0x6A },
	{ 0x4D, 0xD5, 0xAE },
	{ 0x00, 0xD9, 0xFF },
	{ 0x66, 0x66, 0x66 },
	{ 0x0D, 0x0D, 0x0D },
	{ 0x0D, 0x0D, 0x0D },
	{ 0xFF, 0xFF, 0xFF },
	{ 0x84, 0xBF, 0xFF },
	{ 0xBB, 0xBB, 0xFF },
	{ 0xD0, 0xBB, 0xFF },
	{ 0xFF, 0xBF, 0xEA },
	{ 0xFF, 0xBF, 0xCC },
	{ 0xFF, 0xC4, 0xB7 },
	{ 0xFF, 0xCC, 0xAE },
	{ 0xFF, 0xD9, 0xA2 },
	{ 0xCC, 0xE1, 0x99 },
	{ 0xAE, 0xEE, 0xB7 },
	{ 0xAA, 0xF7, 0xEE },
	{ 0xB3, 0xEE, 0xFF },
	{ 0xDD, 0xDD, 0xDD },
	{ 0x11, 0x11, 0x11 },
	{ 0x11, 0x11, 0x11 }
};





typedef struct {
    nes_t tNESConsole;
    frame_t tFrame;
    uint8_t chController[2];
} fce_t;


static NO_INIT fce_t s_tFCE;

#if JEG_DEBUG_SHOW_BACKGROUND == ENABLED
uint_fast8_t debug_fetch_color(uint_fast8_t chColor)
{
    return s_tFCE.tNESConsole.ppu.palette[chColor];
}
#endif

void fce_init(void)
{
    fce_t *ptThis = &s_tFCE;

    nes_hal_init();
    
    memset(&(this.tFrame), 0, sizeof(frame_t));
    
    this.tFrame.hwHeight = SCREEN_HEIGHT;
    this.tFrame.hwWidth = SCREEN_WIDTH;
    this.chController[0] = 0;
    this.chController[1] = 0;
#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == DISABLED
    nes_setup_video(&this.tNESConsole, this.tFrame.chBuffer);
#endif

#if JEG_DEBUG_SHOW_BACKGROUND == ENABLED
    this.tFrame.ptBuffer = &(this.tNESConsole.ppu.tNameAttributeTable[JEG_DEBUG_SHOW_NAMETABLE_INDEX].chBackgroundBuffer);
#endif
}


int32_t fce_load_rom(uint8_t *pchROM, uint_fast32_t wSize)
{
    fce_t *ptThis = &s_tFCE;
    do {
        if (NULL == pchROM) {
            break;
        }
    #if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
        {
            const nes_cfg_t tCFG = {
                &draw_pixels,
                &this.tFrame,
            };
            if (nes_init(&this.tNESConsole, (nes_cfg_t *)&tCFG)) {
                break;
            }
        }
    #else
        nes_init(&this.tNESConsole);
    #endif
        return nes_setup_rom(&this.tNESConsole, pchROM, wSize);
    } while(false);    
    
    return -1;
}



void fce_run(void)
{
    fce_t *ptThis = &s_tFCE;
    
    while(true) {
     
        nes_set_controller(&this.tNESConsole, this.chController[0], this.chController[0]);
#if NES_PROFILINE == ENABLED
        start_counter();

        nes_iterate_frame(&this.tNESConsole);
        int nTimeEmulator = stop_counter();
        if (nes_is_frame_ready(&this.tNESConsole)) {
            start_counter();
            update_frame(&this.tFrame);
            int nTimeRefresh = stop_counter();
            int nTotal = nTimeEmulator+nTimeRefresh;
            
            log_info("NES: %8d %3d \t Refresh: %8d %3d\t %8d %8d ms\r",
                     nTimeEmulator,
                    (nTimeEmulator * 100 + nTotal / 2)/nTotal,
                     nTimeRefresh,
                    (nTimeRefresh*100 + nTotal / 2)/nTotal,
                    nTotal,
                    (int32_t)((uint64_t)((uint64_t)nTotal * 1000) / SystemCoreClock));
        }
#else
        nes_iterate_frame(&this.tNESConsole);
        if (nes_is_frame_ready(&this.tNESConsole)) {
            update_frame(&this.tFrame);
        }
#endif
    }
}
