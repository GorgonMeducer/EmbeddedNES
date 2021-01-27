#include "common.h"

#ifndef PPU_INTERNAL_H
#define PPU_INTERNAL_H



// PPU Memory and State


typedef struct {
    byte PPUCTRL;   // $2000 write only
    byte PPUMASK;   // $2001 write only
    byte PPUSTATUS; // $2002 read only
    byte OAMADDR;   // $2003 write only
    byte OAMDATA;   // $2004
	word PPUSCROLL;
    byte PPUSCROLL_X, PPUSCROLL_Y; // $2005 write only x2
    word PPUADDR;   // $2006 write only x2
    word PPUDATA;   // $2007

    bool scroll_received_x;
    bool addr_received_high_byte;
    bool ready;

    int mirroring, mirroring_xor;

    int x, scanline;
} PPU_STATE;

extern PPU_STATE ppu;

extern byte ppu_latch;
extern bool ppu_sprite_hit_occured;

extern uint_fast16_t ppu_get_real_ram_address(uint_fast16_t address);


// PPU Constants



// Screen State and Rendering

// For sprite-0-hit checks
extern byte ppu_screen_background[264][248];

// Precalculated tile high and low bytes addition for pattern tables
extern byte ppu_l_h_addition_table[256][256][8];
extern byte ppu_l_h_addition_flip_table[256][256][8];

// Draws current screen pixels in ppu_background_pixels & ppu_sprite_pixels and clears them
void ppu_render_screen(void);
void ppu_set_background_color(byte color);



// PPUCTRL Functions

word ppu_base_nametable_address(void);
byte ppu_vram_address_increment(void);
word ppu_sprite_pattern_table_address(void);
word ppu_background_pattern_table_address(void);
byte ppu_sprite_width(void);
byte ppu_sprite_height(void);
bool ppu_generates_nmi(void);



// PPUMASK Functions

bool ppu_renders_grayscale(void);
bool ppu_shows_background_in_leftmost_8px(void);
bool ppu_shows_sprites_in_leftmost_8px(void);
bool ppu_intensifies_reds(void);
bool ppu_intensifies_greens(void);
bool ppu_intensifies_blues(void);
void ppu_set_renders_grayscale(bool yesno);
void ppu_set_shows_background_in_leftmost_8px(bool yesno);
void ppu_set_shows_sprites_in_leftmost_8px(bool yesno);
void ppu_set_shows_background(bool yesno);
void ppu_set_shows_sprites(bool yesno);
void ppu_set_intensifies_reds(bool yesno);
void ppu_set_intensifies_greens(bool yesno);
void ppu_set_intensifies_blues(bool yesno);



// PPUSTATUS Functions

bool ppu_sprite_overflow(void);
bool ppu_sprite_0_hit(void);
bool ppu_in_vblank(void);

void ppu_set_sprite_overflow(bool yesno);
void ppu_set_sprite_0_hit(bool yesno);
void ppu_set_in_vblank(bool yesno);



#endif
