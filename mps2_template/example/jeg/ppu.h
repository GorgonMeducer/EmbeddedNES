#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include "jeg_cfg.h"

typedef struct nes_t nes_t;

typedef int ppu_read_func_t (nes_t *nes, int address); // read data [8bit] from address [16bit]
typedef void ppu_write_func_t (nes_t *nes, int address, int value); // write data [8bit] to address [16bit]
typedef void (*ppu_update_frame_func_t) (void *reference, uint8_t* frame_data, int width, int height);

typedef void ppu_draw_pixel_func_t(void *ptTag, uint_fast8_t chY, uint_fast8_t chX, uint_fast8_t chColor);

typedef struct {
    uint_fast16_t   XScroll     : 5;
    uint_fast16_t   YScroll     : 5;
    uint_fast16_t   XToggleBit  : 1;
    uint_fast16_t   YToggleBit  : 1;
    uint_fast16_t   TileYOffsite: 3;
} v_ram_addr_t;

typedef struct ppu_t {
    nes_t *nes; // reference to nes console

    // ppu state
    uint64_t last_cycle_number; // measured in cpu cycles
    int cycle;
    int scanline;

    uint_fast8_t palette[32];
    union {
        struct {
            uint8_t chNameTable[30][32];
            union {
                struct {
                    uint8_t     Square0 : 2;
                    uint8_t     Square1 : 2;
                    uint8_t     Square2 : 2;
                    uint8_t     Square3 : 2;
                }Group;
                uint8_t chValue;
            }AttributeTable[8][8];
        }tTables[2];
        uint8_t name_table[2048];
    };
    union {
        uint8_t oam_data[256];
        struct {
            uint8_t chY;
            uint8_t chIndex;
            struct {
                uint8_t         ColorH              : 2;
                uint8_t                             : 3;
                uint8_t         Priority            : 1;
                uint8_t         IsFlipHorizontally  : 1;
                uint8_t         IsFlipVertically    : 1;
            } Attributes;
            uint8_t chPosition;
        } SpriteInfo[64];
    };
    bool bOAMUpdated;
  
    struct {
        struct {
            uint_fast8_t chIndex;
            uint_fast8_t chY;
        } List[64] ;

        uint_fast8_t chVisibleCount;
        uint_fast8_t chCurrent; 
    } SpriteYOrderList;
    
    // ppu registers
    union {
        v_ram_addr_t tVAddress;
        uint_fast16_t v; // current vram address (15bit)
    };
    union {
        v_ram_addr_t tTempVAddress;
        uint_fast16_t t; // temporary vram address (15bit)
    };
    int x; // fine x scoll (3bit)
    int w; // toggle bit (1bit)
    int f; // even/odd frame flag (1bit)

    int register_data;

    // background temporary variables
    int name_table_byte;
    int attribute_table_byte;
    int low_tile_byte;
    int high_tile_byte;
    uint64_t tile_data;

    // sprite temporary variables
    int sprite_count;
    uint32_t sprite_patterns      [JEG_MAX_ALLOWED_SPRITES_ON_SINGLE_SCANLINE];
    uint_fast8_t sprite_positions [JEG_MAX_ALLOWED_SPRITES_ON_SINGLE_SCANLINE];
    uint_fast8_t sprite_priorities[JEG_MAX_ALLOWED_SPRITES_ON_SINGLE_SCANLINE];
    uint_fast8_t sprite_indicies  [JEG_MAX_ALLOWED_SPRITES_ON_SINGLE_SCANLINE];

    // memory accessable registers
    int ppuctrl;
    int ppumask;
    int ppustatus;
    int oam_address;
    int buffered_data;

    // memory interface to vram and vrom
    ppu_read_func_t   *read;
    ppu_write_func_t  *write;
  
#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
    ppu_draw_pixel_func_t *fnDrawPixel;
    void *ptTag;
#else
    // frame data interface
    uint8_t *video_frame_data;
#endif

#if JEG_USE_FRAME_SYNC_UP_FLAG  == ENABLED
    bool bFrameReady;
#endif
} ppu_t;

#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
typedef struct {
    nes_t                   *ptNES; 
    ppu_read_func_t         *fnRead; 
    ppu_write_func_t        *fnWrite;
    ppu_draw_pixel_func_t   *fnDrawPixel;
    void *ptTag;
}ppu_cfg_t;

extern bool ppu_init(ppu_t *ppu, ppu_cfg_t *ptCFG);
#else
extern void ppu_init(ppu_t *ppu, nes_t *nes, ppu_read_func_t read, ppu_write_func_t write);
#endif

void ppu_setup_video(ppu_t *ppu, uint8_t *video_frame_data);

void ppu_reset(ppu_t *ppu);

//! \brief read data [8bit] from address [16bit]
extern uint_fast8_t ppu_read(ppu_t *ppu, uint_fast16_t hwAddress) ; 

//! \brief write data [8bit] to address [16bit]
extern void ppu_write(ppu_t *ppu, uint_fast16_t hwAddress, uint_fast8_t chData); 

//! \bridef dedicated PPU DMA access 
extern void ppu_dma_access(ppu_t *ppu, uint_fast8_t chData);

extern int ppu_update(ppu_t *ppu); // update ppu to current cpu cycle, return number of cpu cycles to next frame

#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == DISABLED
extern void ppu_setup_video(ppu_t *ppu, uint8_t *video_frame_data);
#endif

#if JEG_USE_FRAME_SYNC_UP_FLAG  == ENABLED
extern bool ppu_is_frame_ready(ppu_t *ptPPU);
#endif

#endif
