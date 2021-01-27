#include <stdio.h>
#include <stdint.h>
#include "cpu6502.h"
#include "cpu6502_debug.h"

int data[65536]={
  #include "rom.inc"
};

uint_fast8_t _read(void *ref, uint_fast16_t adr) {
  return data[adr];
}

void _write(void *ref, uint_fast16_t adr, uint_fast8_t value) {
  data[adr]=value;
}

int main(int argc, char *argv[]) {
  cpu6502_t cpu;

  cpu6502_init(&cpu, 0, _read, _write);
  cpu6502_reset(&cpu);

  cpu.reg_PC=0x400;

  cpu6502_run(&cpu, 100000000); // run 100 Million Clock Cycles

  if (cpu.reg_PC==0x3399) {
    printf("=== All tests successfull! ===\n");
    return 0;
  }
  else {
    printf("=== Test failed: PC=$%04X ===\n", (unsigned int)cpu.reg_PC);
    return 1;
  }

}
