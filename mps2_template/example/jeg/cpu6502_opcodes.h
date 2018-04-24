#ifndef CPU6502_OPCODES_H
#define CPU6502_OPCODES_H

#include "jeg_cfg.h"

typedef enum address_mode_enum_t {
  ADR_ABSOLUTE, ADR_ABSOLUTE_X, ADR_ABSOLUTE_Y, ADR_ACCUMULATOR, ADR_IMMEDIATE,
  ADR_IMPLIED, ADR_INDEXED_INDIRECT, ADR_INDIRECT, ADR_INDIRECT_INDEXED,
  ADR_RELATIVE, ADR_ZERO_PAGE, ADR_ZERO_PAGE_X, ADR_ZERO_PAGE_Y
} address_mode_enum_t;

typedef enum operation_enum_t {
  OP_ADC, OP_AHX, OP_ALR, OP_ANC, OP_AND, OP_ARR, OP_ASL, OP_AXS, OP_BCC, OP_BCS, OP_BEQ, OP_BIT, OP_BMI, OP_BNE, OP_BPL,
  OP_BRK, OP_BVC, OP_BVS, OP_CLC, OP_CLD, OP_CLI, OP_CLV, OP_CMP, OP_CPX, OP_CPY, OP_DCP, OP_DEC, OP_DEX, OP_DEY, OP_EOR,
  OP_INC, OP_INX, OP_INY, OP_ISC, OP_JMP, OP_JSR, OP_KIL, OP_LAS, OP_LAX, OP_LDA, OP_LDX, OP_LDY, OP_LSR, OP_NOP, OP_ORA,
  OP_PHA, OP_PHP, OP_PLA, OP_PLP, OP_RLA, OP_ROL, OP_ROR, OP_RRA, OP_RTI, OP_RTS, OP_SAX, OP_SBC, OP_SEC, OP_SED, OP_SEI,
  OP_SHX, OP_SHY, OP_SLO, OP_SRE, OP_STA, OP_STX, OP_STY, OP_TAS, OP_TAX, OP_TAY, OP_TSX, OP_TXA, OP_TXS, OP_TYA, OP_XAA
} operation_enum_t;

typedef struct opcode_tbl_entry_t {
  char *mnemonic;
  operation_enum_t operation;
  address_mode_enum_t address_mode;
  int bytes;
  int cycles;
  int page_cross_cycles;
} opcode_tbl_entry_t;

extern const opcode_tbl_entry_t opcode_tbl[];

#endif
