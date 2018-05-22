#ifndef PPU_H
#define PPU_H

#include "common.h"
#include "jeg_cfg.h"

typedef struct nes_t nes_t;

typedef uint_fast8_t ppu_read_func_t (nes_t *, uint_fast16_t);                  //!< read data [8bit] from address [16bit]
typedef void ppu_write_func_t (nes_t *nes, uint_fast16_t, uint_fast8_t);        //!< write data [8bit] to address [16bit]

typedef void ppu_draw_pixel_func_t(void *, uint_fast8_t , uint_fast8_t , uint_fast8_t );


typedef union {
    uint8_t chValue;
    struct {
        uint8_t Low     : 4;
        uint8_t High    : 4;
    };
} compact_dual_pixels_t;

typedef uint8_t nes_screen_buffer_t[240][256];


typedef struct {
    uint_fast16_t   XScroll     : 5;
    uint_fast16_t   YScroll     : 5;
    uint_fast16_t   XToggleBit  : 1;
    uint_fast16_t   YToggleBit  : 1;
    uint_fast16_t   TileYOffsite: 3;
} vram_addr_t;

typedef struct {
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
        };
        uint8_t chBuffer[1024];
    };
    
#if JEG_USE_BACKGROUND_BUFFERING == ENABLED
    compact_dual_pixels_t chBackgroundBuffer[240][128];
    uint_fast32_t wDirtyMatrix[32];                                             //! do not modify it to 30
    bool bRequestRefresh;
#endif
} name_attribute_table_t;

typedef union { 
    struct {
        uint8_t chY;
        uint8_t chIndex;
        union {
            struct {
                uint8_t         ColorH              : 2;
                uint8_t                             : 3;
                uint8_t         Priority            : 1;
                uint8_t         IsFlipHorizontally  : 1;
                uint8_t         IsFlipVertically    : 1;
            }; 
            uint8_t chValue;
        }Attributes;
        uint8_t chPosition;
    }; 
    uint32_t wValue;
}sprite_t;

typedef union {
    uint8_t     chBuffer[256];
    sprite_t    SpriteInfo[64];
} sprite_table_t;

typedef struct ppu_t {
    nes_t *nes; // reference to nes console

    // ppu state
    uint_fast64_t last_cycle_number; // measured in cpu cycles
    int_fast32_t cycle;
    uint_fast16_t scanline;

    uint_fast8_t palette[32];
    union {
#if JEG_USE_4_PHYSICAL_NAME_ATTRIBUTE_TABLES == ENABLED
        name_attribute_table_t tNameAttributeTable[4];
#else
        name_attribute_table_t tNameAttributeTable[2];
#endif
        
    };

    sprite_table_t tSpriteTable;
    
    
    
#if JEG_USE_SPRITE_BUFFER == ENABLED
    sprite_table_t tModifiedSpriteTable;
    uint32_t wSpriteBuffer[64][16];
    bool bRequestRefreshSpriteBuffer;
#endif
    
#if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
    bool bOAMUpdated;
#endif

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
        vram_addr_t tVAddress;
        uint_fast16_t v; // current vram address (15bit)
    };
    union {
        vram_addr_t tTempVAddress;
        uint_fast16_t t; // temporary vram address (15bit)
    };   
    
    uint_fast8_t x; // fine x scoll (3bit)
    uint_fast8_t w; // toggle bit (1bit)
    uint_fast8_t f; // even/odd frame flag (1bit)

    uint_fast8_t register_data;

    // background temporary variables
    uint_fast8_t name_table_byte;
    uint_fast8_t attribute_table_byte;

    uint_fast8_t low_tile_byte;
    uint_fast8_t high_tile_byte;
    uint_fast64_t tile_data;

    // sprite temporary variables
    uint_fast8_t sprite_count;
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

extern uint_fast32_t ppu_update(ppu_t *ppu); // update ppu to current cpu cycle, return number of cpu cycles to next frame

#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == DISABLED
extern void ppu_setup_video(ppu_t *ppu, uint8_t *video_frame_data);
#endif

#if JEG_USE_FRAME_SYNC_UP_FLAG  == ENABLED
extern bool ppu_is_frame_ready(ppu_t *ptPPU);
#endif

#endif
