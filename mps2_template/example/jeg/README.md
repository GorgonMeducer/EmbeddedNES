# JEG Emulation Gathering
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Platform independent catch-up emulation framework with Nintendo NES as main target.

## Dependencies
* C compiler (C99)

## What's working
* CPU 6502 *completed*
* PPU *nearly completed* (`ppu_vbl_nmi` timing test is failing)
* APU *missing*
* Cartridge abstraction *draft is working*
* Supported Mappers: *INES #0*
* Prototype UI using SDL library (for graphics and audio)

## Usefull projects during developlemt
* [github:fogleman/nes](https://github.com/fogleman/nes) (Go, pixel based rendering)
* [github:NJUOS/LiteNES](https://github.com/NJUOS/LiteNES) (C, scanline based rendering)
* [github:amhndu/SimpleNES](https://github.com/amhndu/SimpleNES) (C++, pixel based rendering)

## Test ROMs
Just call `make test` do run all tests. Taken from [NESDev](https://wiki.nesdev.com/w/index.php/Emulator_tests) and [github:christopherpow/nes-test-roms](https://github.com/christopherpow/nes-test-roms).

### Valid
* [Klaus2m5/6502_65C02_functional_tests](https://github.com/Klaus2m5/6502_65C02_functional_tests) by *Klaus Dormann*
* [blargg_ppu_tests_2005.09.15b](https://github.com/christopherpow/nes-test-roms/tree/master/blargg_ppu_tests_2005.09.15b) (without power up test) by *Shay "blargg" Green*
* [branch_timing_tests](https://github.com/christopherpow/nes-test-roms/tree/master/branch_timing_tests) by *Shay "blargg" Green*
* [cpu_dummy_reads](https://github.com/christopherpow/nes-test-roms/tree/master/cpu_dummy_reads) by *Shay "blargg" Green*
* [cpu_exec_space](https://github.com/christopherpow/nes-test-roms/tree/master/cpu_exec_space)(without apu) by *Shay "blargg" Green* and *Joel "bisqwit" Yliluoma*
* [cpu_timing_test6](https://github.com/christopherpow/nes-test-roms/tree/master/cpu_timing_test6) (without unofficial) by *Shay "blargg" Green*

### Not working (yet)
These tests are also not called by `make test`.

* [blargg_nes_cpu_test5](https://github.com/christopherpow/nes-test-roms/tree/master/blargg_nes_cpu_test5) by *Shay "blargg" Green* - Mapper 1 not supported yet
* [cpu_dummy_writes](http://bisqwit.iki.fi/src/nes_tests/cpu_dummy_writes.zip) by *Joel "bisqwit" Yliluoma* - ROM should be not writeable
* [cpu_reset](https://github.com/christopherpow/nes-test-roms/tree/master/cpu_reset) by *Shay "blargg" Green* - no soft reset available yet
* [ppu_vbl_nmi](https://github.com/christopherpow/nes-test-roms/tree/master/ppu_vbl_nmi) by *Shay "blargg" Green* - would be really nice...

## Thanks
* [GorgonMeducer](https://github.com/GorgonMeducer) doing the [EmbeddedNES](https://github.com/GorgonMeducer/EmbeddedNES)-Project for discussing a lot of optimisation patterns, sharing his know how and bring in usefull code!
