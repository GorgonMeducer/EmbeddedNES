.PHONY: all clean

SRCS_CPU6502=cpu6502.c cpu6502_opcodes.c cpu6502_debug.c
SRCS_NES=$(SRCS_CPU6502) ppu.c nes.c cartridge.c
SRCS_UI=$(SRCS_NES) ui.c

all: jeg

jeg: $(SRCS_UI)
	$(CC) $(SRCS_UI) -I. -O3 -o $@ `sdl-config --cflags --libs` -Wall -pedantic $(CFLAGS)

benchmark: $(SRCS_NES) benchmark.c
	$(CC) $(SRCS_NES) benchmark.c -I. -O3 -o $@ -Wall -pedantic -DWITHOUT_DECIMAL_MODE $(CFLAGS)

jeg.exe: $(SRCS_UI)
	i686-w64-mingw32-gcc  $(SRCS_UI) -mwindows -I. -O3 -o $@ -D_GNU_SOURCE=1 -D_REENTRANT -Iwindows/include/SDL -Lwindows/libs -lmingw32 -lSDLmain -lSDL -Wall -pedantic -DWITHOUT_DECIMAL_MODE $(CFLAGS)

test: run_klaus2m5 run_test_roms

test_klaus2m5: $(SRCS_CPU6502) test/klaus2m5/system.c 
	cat test/klaus2m5/6502_functional_test.bin|xxd -i >test/klaus2m5/rom.inc
	$(CC) $(SRCS_CPU6502) -I. -Itest/klaus2m5 test/klaus2m5/system.c -o $@ -O3 -DJEG_USE_6502_DECIMAL_MODE=1

run_klaus2m5: test_klaus2m5
	./test_klaus2m5 test/klaus2m5/6502_functional_test.bin

test_roms: $(SRCS_NES) test/nes_roms/test_roms.c
	$(CC) $(SRCS_NES) -I. test/nes_roms/test_roms.c `sdl-config --cflags --libs` -o $@ -O3 -DWITHOUT_DECIMAL_MODE

run_test_roms: test_roms
	cd test/nes_roms && ../../test_roms -p test.key

clean:
	rm jeg jeg.exe test_klaus2m5 test/klaus2m5/rom.inc test_roms -rf
