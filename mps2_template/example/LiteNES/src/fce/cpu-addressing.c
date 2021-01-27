#include "cpu.h"
#include "cpu-internal.h"
#include "memory.h"

// CPU Addressing Modes

void cpu_address_implied(void)
{
}

void cpu_address_immediate(void)
{
    core.op_value = memory_readb(core.PC);
    core.PC++;
}

void cpu_address_zero_page(void)
{
    core.op_address = memory_readb(core.PC);
    core.op_value = CPU_RAM[core.op_address];
    core.PC++;
}

void cpu_address_zero_page_x(void)
{
    core.op_address = (memory_readb(core.PC) + core.X) & 0xFF;
    core.op_value = CPU_RAM[core.op_address];
    core.PC++;
}

void cpu_address_zero_page_y(void)
{
    core.op_address = (memory_readb(core.PC) + core.Y) & 0xFF;
    core.op_value = CPU_RAM[core.op_address];
    core.PC++;
}

void cpu_address_absolute(void)
{
    core.op_address = memory_readw(core.PC);
    core.op_value = memory_readb(core.op_address);
    core.PC += 2;
}

void cpu_address_absolute_x(void)
{
    core.op_address = memory_readw(core.PC) + core.X;
    core.op_value = memory_readb(core.op_address);
    core.PC += 2;

    if ((core.op_address >> 8) != (core.PC >> 8)) {
        core.op_cycles++;
    }
}

void cpu_address_absolute_y(void)
{
    core.op_address = (memory_readw(core.PC) + core.Y) & 0xFFFF;
    core.op_value = memory_readb(core.op_address);
    core.PC += 2;

    if ((core.op_address >> 8) != (core.PC >> 8)) {
        core.op_cycles++;
    }
}

void cpu_address_relative(void)
{
    core.op_address = memory_readb(core.PC);
    core.PC++;
    if (core.op_address & 0x80) {
        core.op_address -= 0x100;
    }
    core.op_address += core.PC;

    if ((core.op_address >> 8) != (core.PC >> 8)) {
        core.op_cycles++;
    }
}

void cpu_address_indirect(void)
{
    word arg_addr = memory_readw(core.PC);

    // The famous 6502 bug when instead of reading from $C0FF/$C100 it reads from $C0FF/$C000
    if ((arg_addr & 0xFF) == 0xFF) {
        // Buggy code
        core.op_address = (memory_readb(arg_addr & 0xFF00) << 8) + memory_readb(arg_addr);
    } else {
        // Normal code
        core.op_address = memory_readw(arg_addr);
    }
    core.PC += 2;
}

void cpu_address_indirect_x(void)
{
    byte arg_addr = memory_readb(core.PC);
    core.op_address = memory_readw(arg_addr + core.X);//(memory_readb((arg_addr + core.X + 1) & 0xFF) << 8) | memory_readb((arg_addr + core.X) & 0xFF);
    core.op_value = memory_readb(core.op_address);
    core.PC++;
}

void cpu_address_indirect_y(void)
{
    byte arg_addr = memory_readb(core.PC);
    core.op_address = (
                        ((memory_readb((arg_addr + 1) & 0xFF) << 8) | memory_readb(arg_addr))
                        //memory_readw(arg_addr)
                        + core.Y) & 0xFFFF;
    core.op_value = memory_readb(core.op_address);
    core.PC++;

    if ((core.op_address >> 8) != (core.PC >> 8)) {
        core.op_cycles++;
    }
}
