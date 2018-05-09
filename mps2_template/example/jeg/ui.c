#include <stdio.h>
#include <stdint.h>
#include <SDL.h>
#include "nes.h"

// global variables
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
uint32_t pixels[256*240];

static const uint32_t rgb_palette[64] = {
  0x7C7C7C, 0x0000FC, 0x0000BC, 0x4428BC, 0x940084, 0xA80020, 0xA81000, 0x881400,
  0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000, 0x000000, 0x000000,
  0xBCBCBC, 0x0078F8, 0x0058F8, 0x6844FC, 0xD800CC, 0xE40058, 0xF83800, 0xE45C10,
  0xAC7C00, 0x00B800, 0x00A800, 0x00A844, 0x008888, 0x000000, 0x000000, 0x000000,
  0xF8F8F8, 0x3CBCFC, 0x6888FC, 0x9878F8, 0xF878F8, 0xF85898, 0xF87858, 0xFCA044,
  0xF8B800, 0xB8F818, 0x58D854, 0x58F898, 0x00E8D8, 0x787878, 0x000000, 0x000000,
  0xFCFCFC, 0xA4E4FC, 0xB8B8F8, 0xD8B8F8, 0xF8B8F8, 0xF8A4C0, 0xF0D0B0, 0xFCE0A8,
  0xF8D878, 0xD8F878, 0xB8F8B8, 0xB8F8D8, 0x00FCFC, 0xF8D8F8, 0x000000, 0x000000
};

void update_frame(uint8_t* frame_data) {
  for (uint32_t i=0; i<256*240; i++) {
    pixels[i]=rgb_palette[frame_data[i]];
  }
  SDL_UpdateTexture(texture, NULL, pixels, 256*sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
  int result;
  nes_t nes_console;
  SDL_Event event;
  FILE *rom_file;
  uint8_t *rom_data;
  uint32_t rom_size;
  uint8_t video_frame_data[256*240];
  uint8_t controller1=0;
  int wait_ms;
  double next_frame_tick=SDL_GetTicks()+1000.0/60.0;

  // load rom file
  if (argc<2) {
    printf("need rom file as argument\n");
    return 1;
  }

  rom_file=fopen(argv[1], "rb");

  if (rom_file==NULL) {
    printf("not able to open rom file %s\n", argv[1]);
    return 2;
  }

  fseek(rom_file, 0, SEEK_END);
  rom_size=ftell(rom_file);
  fseek(rom_file, 0, SEEK_SET);
  rom_data=malloc(rom_size);
  if (fread(rom_data, 1, rom_size, rom_file)!=rom_size) {
    printf("unable to read data from rom file %s\n", argv[1]);
    return 3;
  }
  fclose(rom_file);

  // init SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    printf("unable to init sdl video\n");
    return 4;
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
 
  window = SDL_CreateWindow("JEG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256*2, 240*2, 0);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  SDL_RenderSetLogicalSize(renderer, 256, 240);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);

  // init nes
  nes_init(&nes_console);
  result=nes_setup_rom(&nes_console, rom_data, rom_size);
  if (result) {
    printf("unable to parse rom file (result:%d)\n", result);
    return 6;
  }

  nes_setup_video(&nes_console, video_frame_data);
  
  int quit = 0;
  uint16_t key_value;

  while(!quit) {
    SDL_PollEvent(&event);
    switch (event.type) {
      case SDL_QUIT:
        quit=1;
        break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        key_value=0;
        switch (event.key.keysym.sym) {
          case SDLK_RIGHT: // controller Right
            key_value=0x80;
            break;
          case SDLK_LEFT: // controller Left
            key_value=0x40;
            break;
          case SDLK_DOWN: // controller Down
            key_value=0x20;
            break;
          case SDLK_UP: // controller Up
            key_value=0x10;
            break;
          case SDLK_q: // controller Start
            key_value=0x08;
            break;
          case SDLK_w: // controller Select
            key_value=0x04;
            break;
          case SDLK_s: // controller B
            key_value=0x02;
            break;
          case SDLK_a: // controller A
            key_value=0x01;
            break;
          default:
            break;
        }

        if (event.type==SDL_KEYDOWN) {
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: 
              quit=1;
              break;
            case SDLK_r: 
              nes_reset(&nes_console);
              break;
            default:
              break;
          }
          controller1|=key_value;
        }
        else {
          controller1&=~key_value;
        }
        break;
      default:
        break;
    }
    nes_set_controller(&nes_console, controller1, 0);
    nes_iterate_frame(&nes_console);
    update_frame(video_frame_data);
    wait_ms= (int)next_frame_tick-SDL_GetTicks();
    if (wait_ms<0) {
      wait_ms=0;
    }
    SDL_Delay(wait_ms);
    next_frame_tick+=1000.0/60.0;
  }

  free(rom_data);
  SDL_Quit();
  
  return 0;
}

