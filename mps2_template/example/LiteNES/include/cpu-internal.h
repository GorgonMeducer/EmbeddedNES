#ifndef __CPU_INTERNAL_H__
#define __CPU_INTERNAL_H__

typedef enum {
    carry_flag     = 0x01,
    zero_flag      = 0x02,
    interrupt_flag = 0x04,
    decimal_flag   = 0x08,
    break_flag     = 0x10,
    unused_flag    = 0x20,
    overflow_flag  = 0x40,
    negative_flag  = 0x80
} cpu_p_flag;

typedef enum {
    carry_bp      = 0,
    zero_bp       = 1,
    interrupt_bp  = 2,
    decimal_bp    = 3,
    break_bp      = 4,
    unused_bp     = 5,
    overflow_bp   = 6,
    negative_bp   = 7
} cpu_p_bp;
/*
typedef struct {
    word PC; // Program Counter,
    byte SP; // Stack Pointer,
    byte A, X, Y; // Registers
    byte P; // Flag Register
} CPU_STATE;
*/
//extern CPU_STATE cpu;

extern byte CPU_RAM[0x8000];

//extern byte op_code;          // Current instruction code
struct __core {
    uint_fast16_t PC; // Program Counter,
    uint_fast8_t  SP; // Stack Pointer,
    byte  A, X, Y; // Registers
    byte  P; // Flag Register;
    
    uint_fast8_t op_value;
    uint_fast16_t op_address;             // Arguments for current instruction
    uint_fast32_t op_cycles;              // Additional instruction cycles used (e.g. when paging occurs)
    uint_fast32_t wStallCycles;
    uint_fast32_t cpu_cycles;
};

extern struct __core core;

//extern uint32_t cpu_cycles;  // Total CPU Cycles Since Power Up (wraps)
/*
extern void (*cpu_op_address_mode[256])();       // Array of address modes
extern void (*cpu_op_handler[256])();            // Array of instruction function pointers
extern bool cpu_op_in_base_instruction_set[256]; // true if instruction is in base 6502 instruction set
extern char *cpu_op_name[256];                   // Instruction names
extern int cpu_op_cycles[256];                   // CPU cycles used by instructions
*/
uint_fast8_t cpu_ram_read(uint_fast16_t address);
void cpu_ram_write(uint_fast16_t address, uint_fast8_t data);

// Interrupt Addresses
uint_fast16_t cpu_nmi_interrupt_address(void);
uint_fast16_t cpu_reset_interrupt_address(void);
uint_fast16_t cpu_irq_interrupt_address(void);

// Updates Zero and Negative flags in P
void cpu_update_zn_flags(byte value);

// If OP_TRACE, print current instruction with all registers into the console
void cpu_trace_instruction(void);

// CPU Adressing Modes
extern void cpu_address_implied(void);
extern void cpu_address_immediate(void);
extern void cpu_address_zero_page(void);
extern void cpu_address_zero_page_x(void);
extern void cpu_address_zero_page_y(void);
extern void cpu_address_absolute(void);
extern void cpu_address_absolute_x(void);
extern void cpu_address_absolute_y(void);
extern void cpu_address_relative(void);
extern void cpu_address_indirect(void);
extern void cpu_address_indirect_x(void);
extern void cpu_address_indirect_y(void);





#endif
