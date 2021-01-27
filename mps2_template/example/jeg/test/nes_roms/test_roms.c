#include <stdio.h>
#include <SDL.h>
#include "nes.h"

// global variables
SDL_Surface *screen; // used in main and update_frame

typedef struct rgb_entry_t {uint8_t red; uint8_t green; uint8_t blue;} rgb_entry_t;

static const rgb_entry_t rgb_palette[64] = {
  {0x80, 0x80, 0x80}, {0x00, 0x00, 0xBB}, {0x37, 0x00, 0xBF}, {0x84, 0x00, 0xA6}, {0xBB, 0x00, 0x6A}, {0xB7, 0x00, 0x1E},
  {0xB3, 0x00, 0x00}, {0x91, 0x26, 0x00}, {0x7B, 0x2B, 0x00}, {0x00, 0x3E, 0x00}, {0x00, 0x48, 0x0D}, {0x00, 0x3C, 0x22},
  {0x00, 0x2F, 0x66}, {0x00, 0x00, 0x00}, {0x05, 0x05, 0x05}, {0x05, 0x05, 0x05}, {0xC8, 0xC8, 0xC8}, {0x00, 0x59, 0xFF},
  {0x44, 0x3C, 0xFF}, {0xB7, 0x33, 0xCC}, {0xFF, 0x33, 0xAA}, {0xFF, 0x37, 0x5E}, {0xFF, 0x37, 0x1A}, {0xD5, 0x4B, 0x00},
  {0xC4, 0x62, 0x00}, {0x3C, 0x7B, 0x00}, {0x1E, 0x84, 0x15}, {0x00, 0x95, 0x66}, {0x00, 0x84, 0xC4}, {0x11, 0x11, 0x11},
  {0x09, 0x09, 0x09}, {0x09, 0x09, 0x09}, {0xFF, 0xFF, 0xFF}, {0x00, 0x95, 0xFF}, {0x6F, 0x84, 0xFF}, {0xD5, 0x6F, 0xFF},
  {0xFF, 0x77, 0xCC}, {0xFF, 0x6F, 0x99}, {0xFF, 0x7B, 0x59}, {0xFF, 0x91, 0x5F}, {0xFF, 0xA2, 0x33}, {0xA6, 0xBF, 0x00},
  {0x51, 0xD9, 0x6A}, {0x4D, 0xD5, 0xAE}, {0x00, 0xD9, 0xFF}, {0x66, 0x66, 0x66}, {0x0D, 0x0D, 0x0D}, {0x0D, 0x0D, 0x0D},
  {0xFF, 0xFF, 0xFF}, {0x84, 0xBF, 0xFF}, {0xBB, 0xBB, 0xFF}, {0xD0, 0xBB, 0xFF}, {0xFF, 0xBF, 0xEA}, {0xFF, 0xBF, 0xCC},
  {0xFF, 0xC4, 0xB7}, {0xFF, 0xCC, 0xAE}, {0xFF, 0xD9, 0xA2}, {0xCC, 0xE1, 0x99}, {0xAE, 0xEE, 0xB7}, {0xAA, 0xF7, 0xEE},
  {0xB3, 0xEE, 0xFF}, {0xDD, 0xDD, 0xDD}, {0x11, 0x11, 0x11}, {0x11, 0x11, 0x11}
};

uint32_t sdl_palette[64];

static const uint8_t b64_table[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};


void update_frame(uint8_t* frame_data, int width, int height) {
  uint32_t *pixmem=screen->pixels;

  if(SDL_MUSTLOCK(screen)) {
    if(SDL_LockSurface(screen) < 0) {
      return;
    }
  }
  
  for(int p = 0; p < width*height; p++ )  {
    *pixmem=sdl_palette[*frame_data];
    pixmem++;
    frame_data++;
  }

  if(SDL_MUSTLOCK(screen)) {
    SDL_UnlockSurface(screen);
  }
  
  SDL_Flip(screen); 
}

typedef enum test_mode_t {RECORD, PLAY, ACCEPT} test_mode_t;

uint8_t *rom_data=0;

int load_rom(nes_t *nes, char *filename) {
  FILE *rom_file;
  uint32_t rom_size=0;
  int result;
  uint8_t key_value;

  rom_file=fopen(filename, "rb");

  if (rom_file==NULL) {
    printf("not able to open rom file %s\n", filename);
    return 1;
  }

  fseek(rom_file, 0, SEEK_END);
  rom_size=ftell(rom_file);
  fseek(rom_file, 0, SEEK_SET);
  if (rom_data) { 
    free(rom_data);
  }
  rom_data=malloc(rom_size);
  if (fread(rom_data, 1, rom_size, rom_file)!=rom_size) {
    printf("unable to read data from rom file %s\n", filename);
    return 2;
  }
  fclose(rom_file);

  result=nes_setup_rom(nes, rom_data, rom_size);
  if (result) {
    printf("unable to parse rom file (result:%d)\n", result);
    return 3;
  }

  return 0;
}

void free_rom() {
  if (rom_data) {
    free(rom_data);
    rom_data=0;
  }
}

int main(int argc, char* argv[]) {
  test_mode_t test_mode;
  nes_t nes_console;
  SDL_Event event;
  FILE *keypress_file;
  uint8_t nes_frame_data[256*240];
  uint8_t test_frame_data[256*240*3];
  uint8_t controller1=0;
  int key_value=0;

  // parse command line arguments
  if (argc<3) {
    printf("%s [-r|-p|-a] [keypress file]\n", argv[0]);
    return 1;
  }

  if (strcmp(argv[1], "-r")==0) {
    keypress_file=fopen(argv[2], "w");
    test_mode=RECORD;
  }
  else if (strcmp(argv[1], "-p")==0) {
    keypress_file=fopen(argv[2], "r");
    test_mode=PLAY;
  }
  else if (strcmp(argv[1], "-a")==0) {
    keypress_file=fopen(argv[2], "r");
    test_mode=ACCEPT;
  }
  else {
    printf("unknown test mode (\"%s\")\n", argv[2]);
    return 4;
  }

  // init SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    printf("unable to init sdl video\n");
    return 5;
  }
   
  if (!(screen = SDL_SetVideoMode(test_mode==RECORD?256:256*3, 240, 32, SDL_HWSURFACE))) {
    SDL_Quit();
    printf("unable to set sdl video mode\n");
    return 6;
  }

  for(int idx=0; idx<64; idx++) {
    sdl_palette[idx]=SDL_MapRGB(screen->format, rgb_palette[idx].red, rgb_palette[idx].green, rgb_palette[idx].blue);
  }

  nes_init(&nes_console);
  nes_setup_video(&nes_console, nes_frame_data);
  
  int quit = 0;
  
  if (test_mode==PLAY) {
    int color;
    int valid;
    char line[256*240+3];
    while(1) {
      if (fgets(line, 256*240+3, keypress_file)==0) {
        printf("error reading keypress file\n");
        break;
      }

      line[strcspn(line, "\r\n")]=0;

      switch (line[0]) {
        case 'K': // simulate key press and advance one frame
          sscanf(line+1, "%x", &key_value);
          nes_set_controller(&nes_console, key_value, 0);
          nes_iterate_frame(&nes_console);
          break;
        case 'S': // check screenshot
          valid=1;
          for(int y=0; y<240; y++) {
            for(int x=0; x<256; x++) {
              test_frame_data[y*256*3+x]=nes_frame_data[y*256+x];
              for (color=0; b64_table[color]!=line[1+y*256+x] && color<64; color++);
              test_frame_data[y*256*3+x+256]=color;
              test_frame_data[y*256*3+x+512]=(color!=nes_frame_data[y*256+x])?6:15; // red if unequal, black if equal
              if (color!=nes_frame_data[y*256+x]) {
                valid=0;
              }
            }
          }
          update_frame(test_frame_data, 256*3, 240);
          if (valid==0) {
            quit=0;
            while(!quit) {
              SDL_WaitEvent(&event);
              switch (event.type) {
                case SDL_QUIT:
                  quit=1;
                  break;
                case SDL_KEYDOWN:
                  if (event.key.keysym.sym==SDLK_ESCAPE || event.key.keysym.sym==SDLK_RETURN) {
                    quit=1;
                  }
                  break;
              }
            }
          }
          break;
        case 'L': // load rom
          load_rom(&nes_console, line+1);
          break;
        case 'R': // reset
          nes_reset(&nes_console);
          break;
      }
      if (line[0]=='Q') {
        break;
      }
    }
  }
  else {
    while(!quit) {
      char filename[512];

      SDL_WaitEvent(&event);
      switch (event.type) {
        case SDL_QUIT:
          quit=1;
          break;
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_RIGHT: // controller Right
              key_value^=0x80;
              break;
            case SDLK_LEFT: // controller Left
              key_value^=0x40;
              break;
            case SDLK_DOWN: // controller Down
              key_value^=0x20;
              break;
            case SDLK_UP: // controller Up
              key_value^=0x10;
              break;
            case SDLK_q: // controller Start
              key_value^=0x08;
              break;
            case SDLK_w: // controller Select
              key_value^=0x04;
              break;
            case SDLK_s: // controller B
              key_value^=0x02;
              break;
            case SDLK_a: // controller A
              key_value^=0x01;
              break;
            case SDLK_n: // next frame
              fprintf(keypress_file, "K%02x\n", key_value);
              nes_set_controller(&nes_console, key_value, 0);
              nes_iterate_frame(&nes_console);
              update_frame(nes_frame_data, 256, 240);
              break;
            case SDLK_l: // load rom
              printf("load rom file:");
              while(scanf("%512s", filename)!=1);
              load_rom(&nes_console, filename);
              fprintf(keypress_file, "L%s\n", filename);
              break;
            case SDLK_r: // reset
              fprintf(keypress_file, "R\n");
              nes_reset(&nes_console);
              break;
            case SDLK_d: // screen dump
              fprintf(keypress_file, "S");
              for (int i=0; i<256*240; i++) {
                fprintf(keypress_file, "%c", b64_table[nes_frame_data[i]]);
              }
              fprintf(keypress_file, "\n");
              break;
            case SDLK_ESCAPE: // quit
              fprintf(keypress_file, "Q\n");
              quit=1;
              break;
            default:
              break;
          }
          printf("%6s %6s %6s %6s %6s %6s %6s %6s\n", key_value&0x80?"Right":"", key_value&0x40?"Left":"", key_value&0x20?"Down":"",
            key_value&0x10?"Up":"", key_value&0x08?"Start":"", key_value&0x04?"Select":"", key_value&0x02?"B":"", key_value&0x01?"A":"");
        default:
          break;
      }
    }
  }

  free_rom();
  fclose(keypress_file);
  SDL_Quit();
  
  return 0;
}

