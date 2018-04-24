#include "cpu6502_opcodes.h"
#include "jeg_cfg.h"

const opcode_tbl_entry_t opcode_tbl[256]={
  // mnemonic   |address mode            |cycles
  //    |operation                    |bytes|page crossed cycles
  {"BRK", OP_BRK, ADR_IMPLIED         , 1, 7, 0}, // 0x00
  {"ORA", OP_ORA, ADR_INDEXED_INDIRECT, 2, 6, 0}, // 0x01
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0x02
  {"SLO", OP_SLO, ADR_INDEXED_INDIRECT, 0, 8, 0}, // 0x03
  {"NOP", OP_NOP, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x04
  {"ORA", OP_ORA, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x05
  {"ASL", OP_ASL, ADR_ZERO_PAGE       , 2, 5, 0}, // 0x06
  {"SLO", OP_SLO, ADR_ZERO_PAGE       , 0, 5, 0}, // 0x07
  {"PHP", OP_PHP, ADR_IMPLIED         , 1, 3, 0}, // 0x08
  {"ORA", OP_ORA, ADR_IMMEDIATE       , 2, 2, 0}, // 0x09
  {"ASL", OP_ASL, ADR_ACCUMULATOR     , 1, 2, 0}, // 0x0A
  {"ANC", OP_ANC, ADR_IMMEDIATE       , 0, 2, 0}, // 0x0B
  {"NOP", OP_NOP, ADR_ABSOLUTE        , 3, 4, 0}, // 0x0C
  {"ORA", OP_ORA, ADR_ABSOLUTE        , 3, 4, 0}, // 0x0D
  {"ASL", OP_ASL, ADR_ABSOLUTE        , 3, 6, 0}, // 0x0E
  {"SLO", OP_SLO, ADR_ABSOLUTE        , 0, 6, 0}, // 0x0F
  {"BPL", OP_BPL, ADR_RELATIVE        , 2, 2, 1}, // 0x10
  {"ORA", OP_ORA, ADR_INDIRECT_INDEXED, 2, 5, 1}, // 0x11
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0x12
  {"SLO", OP_SLO, ADR_INDIRECT_INDEXED, 0, 8, 0}, // 0x13
  {"NOP", OP_NOP, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0x14
  {"ORA", OP_ORA, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0x15
  {"ASL", OP_ASL, ADR_ZERO_PAGE_X     , 2, 6, 0}, // 0x16
  {"SLO", OP_SLO, ADR_ZERO_PAGE_X     , 0, 6, 0}, // 0x17
  {"CLC", OP_CLC, ADR_IMPLIED         , 1, 2, 0}, // 0x18
  {"ORA", OP_ORA, ADR_ABSOLUTE_Y      , 3, 4, 1}, // 0x19
  {"NOP", OP_NOP, ADR_IMPLIED         , 1, 2, 0}, // 0x1A
  {"SLO", OP_SLO, ADR_ABSOLUTE_Y      , 0, 7, 0}, // 0x1B
  {"NOP", OP_NOP, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0x1C
  {"ORA", OP_ORA, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0x1D
  {"ASL", OP_ASL, ADR_ABSOLUTE_X      , 3, 7, 0}, // 0x1E
  {"SLO", OP_SLO, ADR_ABSOLUTE_X      , 0, 7, 0}, // 0x1F
  {"JSR", OP_JSR, ADR_ABSOLUTE        , 3, 6, 0}, // 0x20
  {"AND", OP_AND, ADR_INDEXED_INDIRECT, 2, 6, 0}, // 0x21
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0x22
  {"RLA", OP_RLA, ADR_INDEXED_INDIRECT, 0, 8, 0}, // 0x23
  {"BIT", OP_BIT, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x24
  {"AND", OP_AND, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x25
  {"ROL", OP_ROL, ADR_ZERO_PAGE       , 2, 5, 0}, // 0x26
  {"RLA", OP_RLA, ADR_ZERO_PAGE       , 0, 5, 0}, // 0x27
  {"PLP", OP_PLP, ADR_IMPLIED         , 1, 4, 0}, // 0x28
  {"AND", OP_AND, ADR_IMMEDIATE       , 2, 2, 0}, // 0x29
  {"ROL", OP_ROL, ADR_ACCUMULATOR     , 1, 2, 0}, // 0x2A
  {"ANC", OP_ANC, ADR_IMMEDIATE       , 0, 2, 0}, // 0x2B
  {"BIT", OP_BIT, ADR_ABSOLUTE        , 3, 4, 0}, // 0x2C
  {"AND", OP_AND, ADR_ABSOLUTE        , 3, 4, 0}, // 0x2D
  {"ROL", OP_ROL, ADR_ABSOLUTE        , 3, 6, 0}, // 0x2E
  {"RLA", OP_RLA, ADR_ABSOLUTE        , 0, 6, 0}, // 0x2F
  {"BMI", OP_BMI, ADR_RELATIVE        , 2, 2, 1}, // 0x30
  {"AND", OP_AND, ADR_INDIRECT_INDEXED, 2, 5, 1}, // 0x31
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0x32
  {"RLA", OP_RLA, ADR_INDIRECT_INDEXED, 0, 8, 0}, // 0x33
  {"NOP", OP_NOP, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0x34
  {"AND", OP_AND, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0x35
  {"ROL", OP_ROL, ADR_ZERO_PAGE_X     , 2, 6, 0}, // 0x36
  {"RLA", OP_RLA, ADR_ZERO_PAGE_X     , 0, 6, 0}, // 0x37
  {"SEC", OP_SEC, ADR_IMPLIED         , 1, 2, 0}, // 0x38
  {"AND", OP_AND, ADR_ABSOLUTE_Y      , 3, 4, 1}, // 0x39
  {"NOP", OP_NOP, ADR_IMPLIED         , 1, 2, 0}, // 0x3A
  {"RLA", OP_RLA, ADR_ABSOLUTE_Y      , 0, 7, 0}, // 0x3B
  {"NOP", OP_NOP, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0x3C
  {"AND", OP_AND, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0x3D
  {"ROL", OP_ROL, ADR_ABSOLUTE_X      , 3, 7, 0}, // 0x3E
  {"RLA", OP_RLA, ADR_ABSOLUTE_X      , 0, 7, 0}, // 0x3F
  {"RTI", OP_RTI, ADR_IMPLIED         , 1, 6, 0}, // 0x40
  {"EOR", OP_EOR, ADR_INDEXED_INDIRECT, 2, 6, 0}, // 0x41
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0x42
  {"SRE", OP_SRE, ADR_INDEXED_INDIRECT, 0, 8, 0}, // 0x43
  {"NOP", OP_NOP, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x44
  {"EOR", OP_EOR, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x45
  {"LSR", OP_LSR, ADR_ZERO_PAGE       , 2, 5, 0}, // 0x46
  {"SRE", OP_SRE, ADR_ZERO_PAGE       , 0, 5, 0}, // 0x47
  {"PHA", OP_PHA, ADR_IMPLIED         , 1, 3, 0}, // 0x48
  {"EOR", OP_EOR, ADR_IMMEDIATE       , 2, 2, 0}, // 0x49
  {"LSR", OP_LSR, ADR_ACCUMULATOR     , 1, 2, 0}, // 0x4A
  {"ALR", OP_ALR, ADR_IMMEDIATE       , 0, 2, 0}, // 0x4B
  {"JMP", OP_JMP, ADR_ABSOLUTE        , 3, 3, 0}, // 0x4C
  {"EOR", OP_EOR, ADR_ABSOLUTE        , 3, 4, 0}, // 0x4D
  {"LSR", OP_LSR, ADR_ABSOLUTE        , 3, 6, 0}, // 0x4E
  {"SRE", OP_SRE, ADR_ABSOLUTE        , 0, 6, 0}, // 0x4F
  {"BVC", OP_BVC, ADR_RELATIVE        , 2, 2, 1}, // 0x50
  {"EOR", OP_EOR, ADR_INDIRECT_INDEXED, 2, 5, 1}, // 0x51
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0x52
  {"SRE", OP_SRE, ADR_INDIRECT_INDEXED, 0, 8, 0}, // 0x53
  {"NOP", OP_NOP, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0x54
  {"EOR", OP_EOR, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0x55
  {"LSR", OP_LSR, ADR_ZERO_PAGE_X     , 2, 6, 0}, // 0x56
  {"SRE", OP_SRE, ADR_ZERO_PAGE_X     , 0, 6, 0}, // 0x57
  {"CLI", OP_CLI, ADR_IMPLIED         , 1, 2, 0}, // 0x58
  {"EOR", OP_EOR, ADR_ABSOLUTE_Y      , 3, 4, 1}, // 0x59
  {"NOP", OP_NOP, ADR_IMPLIED         , 1, 2, 0}, // 0x5A
  {"SRE", OP_SRE, ADR_ABSOLUTE_Y      , 0, 7, 0}, // 0x5B
  {"NOP", OP_NOP, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0x5C
  {"EOR", OP_EOR, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0x5D
  {"LSR", OP_LSR, ADR_ABSOLUTE_X      , 3, 7, 0}, // 0x5E
  {"SRE", OP_SRE, ADR_ABSOLUTE_X      , 0, 7, 0}, // 0x5F
  {"RTS", OP_RTS, ADR_IMPLIED         , 1, 6, 0}, // 0x60
  {"ADC", OP_ADC, ADR_INDEXED_INDIRECT, 2, 6, 0}, // 0x61
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0x62
  {"RRA", OP_RRA, ADR_INDEXED_INDIRECT, 0, 8, 0}, // 0x63
  {"NOP", OP_NOP, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x64
  {"ADC", OP_ADC, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x65
  {"ROR", OP_ROR, ADR_ZERO_PAGE       , 2, 5, 0}, // 0x66
  {"RRA", OP_RRA, ADR_ZERO_PAGE       , 0, 5, 0}, // 0x67
  {"PLA", OP_PLA, ADR_IMPLIED         , 1, 4, 0}, // 0x68
  {"ADC", OP_ADC, ADR_IMMEDIATE       , 2, 2, 0}, // 0x69
  {"ROR", OP_ROR, ADR_ACCUMULATOR     , 1, 2, 0}, // 0x6A
  {"ARR", OP_ARR, ADR_IMMEDIATE       , 0, 2, 0}, // 0x6B
  {"JMP", OP_JMP, ADR_INDIRECT        , 3, 5, 0}, // 0x6C
  {"ADC", OP_ADC, ADR_ABSOLUTE        , 3, 4, 0}, // 0x6D
  {"ROR", OP_ROR, ADR_ABSOLUTE        , 3, 6, 0}, // 0x6E
  {"RRA", OP_RRA, ADR_ABSOLUTE        , 0, 6, 0}, // 0x6F
  {"BVS", OP_BVS, ADR_RELATIVE        , 2, 2, 1}, // 0x70
  {"ADC", OP_ADC, ADR_INDIRECT_INDEXED, 2, 5, 1}, // 0x71
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0x72
  {"RRA", OP_RRA, ADR_INDIRECT_INDEXED, 0, 8, 0}, // 0x73
  {"NOP", OP_NOP, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0x74
  {"ADC", OP_ADC, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0x75
  {"ROR", OP_ROR, ADR_ZERO_PAGE_X     , 2, 6, 0}, // 0x76
  {"RRA", OP_RRA, ADR_ZERO_PAGE_X     , 0, 6, 0}, // 0x77
  {"SEI", OP_SEI, ADR_IMPLIED         , 1, 2, 0}, // 0x78
  {"ADC", OP_ADC, ADR_ABSOLUTE_Y      , 3, 4, 1}, // 0x79
  {"NOP", OP_NOP, ADR_IMPLIED         , 1, 2, 0}, // 0x7A
  {"RRA", OP_RRA, ADR_ABSOLUTE_Y      , 0, 7, 0}, // 0x7B
  {"NOP", OP_NOP, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0x7C
  {"ADC", OP_ADC, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0x7D
  {"ROR", OP_ROR, ADR_ABSOLUTE_X      , 3, 7, 0}, // 0x7E
  {"RRA", OP_RRA, ADR_ABSOLUTE_X      , 0, 7, 0}, // 0x7F
  {"NOP", OP_NOP, ADR_IMMEDIATE       , 2, 2, 0}, // 0x80
  {"STA", OP_STA, ADR_INDEXED_INDIRECT, 2, 6, 0}, // 0x81
  {"NOP", OP_NOP, ADR_IMMEDIATE       , 0, 2, 0}, // 0x82
  {"SAX", OP_SAX, ADR_INDEXED_INDIRECT, 0, 6, 0}, // 0x83
  {"STY", OP_STY, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x84
  {"STA", OP_STA, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x85
  {"STX", OP_STX, ADR_ZERO_PAGE       , 2, 3, 0}, // 0x86
  {"SAX", OP_SAX, ADR_ZERO_PAGE       , 0, 3, 0}, // 0x87
  {"DEY", OP_DEY, ADR_IMPLIED         , 1, 2, 0}, // 0x88
  {"NOP", OP_NOP, ADR_IMMEDIATE       , 0, 2, 0}, // 0x89
  {"TXA", OP_TXA, ADR_IMPLIED         , 1, 2, 0}, // 0x8A
  {"XAA", OP_XAA, ADR_IMMEDIATE       , 0, 2, 0}, // 0x8B
  {"STY", OP_STY, ADR_ABSOLUTE        , 3, 4, 0}, // 0x8C
  {"STA", OP_STA, ADR_ABSOLUTE        , 3, 4, 0}, // 0x8D
  {"STX", OP_STX, ADR_ABSOLUTE        , 3, 4, 0}, // 0x8E
  {"SAX", OP_SAX, ADR_ABSOLUTE        , 0, 4, 0}, // 0x8F
  {"BCC", OP_BCC, ADR_RELATIVE        , 2, 2, 1}, // 0x90
  {"STA", OP_STA, ADR_INDIRECT_INDEXED, 2, 6, 0}, // 0x91
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0x92
  {"AHX", OP_AHX, ADR_INDIRECT_INDEXED, 0, 6, 0}, // 0x93
  {"STY", OP_STY, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0x94
  {"STA", OP_STA, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0x95
  {"STX", OP_STX, ADR_ZERO_PAGE_Y     , 2, 4, 0}, // 0x96
  {"SAX", OP_SAX, ADR_ZERO_PAGE_Y     , 0, 4, 0}, // 0x97
  {"TYA", OP_TYA, ADR_IMPLIED         , 1, 2, 0}, // 0x98
  {"STA", OP_STA, ADR_ABSOLUTE_Y      , 3, 5, 0}, // 0x99
  {"TXS", OP_TXS, ADR_IMPLIED         , 1, 2, 0}, // 0x9A
  {"TAS", OP_TAS, ADR_ABSOLUTE_Y      , 0, 5, 0}, // 0x9B
  {"SHY", OP_SHY, ADR_ABSOLUTE_X      , 0, 5, 0}, // 0x9C
  {"STA", OP_STA, ADR_ABSOLUTE_X      , 3, 5, 0}, // 0x9D
  {"SHX", OP_SHX, ADR_ABSOLUTE_Y      , 0, 5, 0}, // 0x9E
  {"AHX", OP_AHX, ADR_ABSOLUTE_Y      , 0, 5, 0}, // 0x9F
  {"LDY", OP_LDY, ADR_IMMEDIATE       , 2, 2, 0}, // 0xA0
  {"LDA", OP_LDA, ADR_INDEXED_INDIRECT, 2, 6, 0}, // 0xA1
  {"LDX", OP_LDX, ADR_IMMEDIATE       , 2, 2, 0}, // 0xA2
  {"LAX", OP_LAX, ADR_INDEXED_INDIRECT, 0, 6, 0}, // 0xA3
  {"LDY", OP_LDY, ADR_ZERO_PAGE       , 2, 3, 0}, // 0xA4
  {"LDA", OP_LDA, ADR_ZERO_PAGE       , 2, 3, 0}, // 0xA5
  {"LDX", OP_LDX, ADR_ZERO_PAGE       , 2, 3, 0}, // 0xA6
  {"LAX", OP_LAX, ADR_ZERO_PAGE       , 0, 3, 0}, // 0xA7
  {"TAY", OP_TAY, ADR_IMPLIED         , 1, 2, 0}, // 0xA8
  {"LDA", OP_LDA, ADR_IMMEDIATE       , 2, 2, 0}, // 0xA9
  {"TAX", OP_TAX, ADR_IMPLIED         , 1, 2, 0}, // 0xAA
  {"LAX", OP_LAX, ADR_IMMEDIATE       , 0, 2, 0}, // 0xAB
  {"LDY", OP_LDY, ADR_ABSOLUTE        , 3, 4, 0}, // 0xAC
  {"LDA", OP_LDA, ADR_ABSOLUTE        , 3, 4, 0}, // 0xAD
  {"LDX", OP_LDX, ADR_ABSOLUTE        , 3, 4, 0}, // 0xAE
  {"LAX", OP_LAX, ADR_ABSOLUTE        , 0, 4, 0}, // 0xAF
  {"BCS", OP_BCS, ADR_RELATIVE        , 2, 2, 1}, // 0xB0
  {"LDA", OP_LDA, ADR_INDIRECT_INDEXED, 2, 5, 1}, // 0xB1
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0xB2
  {"LAX", OP_LAX, ADR_INDIRECT_INDEXED, 0, 5, 1}, // 0xB3
  {"LDY", OP_LDY, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0xB4
  {"LDA", OP_LDA, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0xB5
  {"LDX", OP_LDX, ADR_ZERO_PAGE_Y     , 2, 4, 0}, // 0xB6
  {"LAX", OP_LAX, ADR_ZERO_PAGE_Y     , 0, 4, 0}, // 0xB7
  {"CLV", OP_CLV, ADR_IMPLIED         , 1, 2, 0}, // 0xB8
  {"LDA", OP_LDA, ADR_ABSOLUTE_Y      , 3, 4, 1}, // 0xB9
  {"TSX", OP_TSX, ADR_IMPLIED         , 1, 2, 0}, // 0xBA
  {"LAS", OP_LAS, ADR_ABSOLUTE_Y      , 0, 4, 1}, // 0xBB
  {"LDY", OP_LDY, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0xBC
  {"LDA", OP_LDA, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0xBD
  {"LDX", OP_LDX, ADR_ABSOLUTE_Y      , 3, 4, 1}, // 0xBE
  {"LAX", OP_LAX, ADR_ABSOLUTE_Y      , 0, 4, 1}, // 0xBF
  {"CPY", OP_CPY, ADR_IMMEDIATE       , 2, 2, 0}, // 0xC0
  {"CMP", OP_CMP, ADR_INDEXED_INDIRECT, 2, 6, 0}, // 0xC1
  {"NOP", OP_NOP, ADR_IMMEDIATE       , 0, 2, 0}, // 0xC2
  {"DCP", OP_DCP, ADR_INDEXED_INDIRECT, 0, 8, 0}, // 0xC3
  {"CPY", OP_CPY, ADR_ZERO_PAGE       , 2, 3, 0}, // 0xC4
  {"CMP", OP_CMP, ADR_ZERO_PAGE       , 2, 3, 0}, // 0xC5
  {"DEC", OP_DEC, ADR_ZERO_PAGE       , 2, 5, 0}, // 0xC6
  {"DCP", OP_DCP, ADR_ZERO_PAGE       , 0, 5, 0}, // 0xC7
  {"INY", OP_INY, ADR_IMPLIED         , 1, 2, 0}, // 0xC8
  {"CMP", OP_CMP, ADR_IMMEDIATE       , 2, 2, 0}, // 0xC9
  {"DEX", OP_DEX, ADR_IMPLIED         , 1, 2, 0}, // 0xCA
  {"AXS", OP_AXS, ADR_IMMEDIATE       , 0, 2, 0}, // 0xCB
  {"CPY", OP_CPY, ADR_ABSOLUTE        , 3, 4, 0}, // 0xCC
  {"CMP", OP_CMP, ADR_ABSOLUTE        , 3, 4, 0}, // 0xCD
  {"DEC", OP_DEC, ADR_ABSOLUTE        , 3, 6, 0}, // 0xCE
  {"DCP", OP_DCP, ADR_ABSOLUTE        , 0, 6, 0}, // 0xCF
  {"BNE", OP_BNE, ADR_RELATIVE        , 2, 2, 1}, // 0xD0
  {"CMP", OP_CMP, ADR_INDIRECT_INDEXED, 2, 5, 1}, // 0xD1
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0xD2
  {"DCP", OP_DCP, ADR_INDIRECT_INDEXED, 0, 8, 0}, // 0xD3
  {"NOP", OP_NOP, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0xD4
  {"CMP", OP_CMP, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0xD5
  {"DEC", OP_DEC, ADR_ZERO_PAGE_X     , 2, 6, 0}, // 0xD6
  {"DCP", OP_DCP, ADR_ZERO_PAGE_X     , 0, 6, 0}, // 0xD7
  {"CLD", OP_CLD, ADR_IMPLIED         , 1, 2, 0}, // 0xD8
  {"CMP", OP_CMP, ADR_ABSOLUTE_Y      , 3, 4, 1}, // 0xD9
  {"NOP", OP_NOP, ADR_IMPLIED         , 1, 2, 0}, // 0xDA
  {"DCP", OP_DCP, ADR_ABSOLUTE_Y      , 0, 7, 0}, // 0xDB
  {"NOP", OP_NOP, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0xDC
  {"CMP", OP_CMP, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0xDD
  {"DEC", OP_DEC, ADR_ABSOLUTE_X      , 3, 7, 0}, // 0xDE
  {"DCP", OP_DCP, ADR_ABSOLUTE_X      , 0, 7, 0}, // 0xDF
  {"CPX", OP_CPX, ADR_IMMEDIATE       , 2, 2, 0}, // 0xE0
  {"SBC", OP_SBC, ADR_INDEXED_INDIRECT, 2, 6, 0}, // 0xE1
  {"NOP", OP_NOP, ADR_IMMEDIATE       , 0, 2, 0}, // 0xE2
  {"ISC", OP_ISC, ADR_INDEXED_INDIRECT, 0, 8, 0}, // 0xE3
  {"CPX", OP_CPX, ADR_ZERO_PAGE       , 2, 3, 0}, // 0xE4
  {"SBC", OP_SBC, ADR_ZERO_PAGE       , 2, 3, 0}, // 0xE5
  {"INC", OP_INC, ADR_ZERO_PAGE       , 2, 5, 0}, // 0xE6
  {"ISC", OP_ISC, ADR_ZERO_PAGE       , 0, 5, 0}, // 0xE7
  {"INX", OP_INX, ADR_IMPLIED         , 1, 2, 0}, // 0xE8
  {"SBC", OP_SBC, ADR_IMMEDIATE       , 2, 2, 0}, // 0xE9
  {"NOP", OP_NOP, ADR_IMPLIED         , 1, 2, 0}, // 0xEA
  {"SBC", OP_SBC, ADR_IMMEDIATE       , 0, 2, 0}, // 0xEB
  {"CPX", OP_CPX, ADR_ABSOLUTE        , 3, 4, 0}, // 0xEC
  {"SBC", OP_SBC, ADR_ABSOLUTE        , 3, 4, 0}, // 0xED
  {"INC", OP_INC, ADR_ABSOLUTE        , 3, 6, 0}, // 0xEE
  {"ISC", OP_ISC, ADR_ABSOLUTE        , 0, 6, 0}, // 0xEF
  {"BEQ", OP_BEQ, ADR_RELATIVE        , 2, 2, 1}, // 0xF0
  {"SBC", OP_SBC, ADR_INDIRECT_INDEXED, 2, 5, 1}, // 0xF1
  {"KIL", OP_KIL, ADR_IMPLIED         , 0, 2, 0}, // 0xF2
  {"ISC", OP_ISC, ADR_INDIRECT_INDEXED, 0, 8, 0}, // 0xF3
  {"NOP", OP_NOP, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0xF4
  {"SBC", OP_SBC, ADR_ZERO_PAGE_X     , 2, 4, 0}, // 0xF5
  {"INC", OP_INC, ADR_ZERO_PAGE_X     , 2, 6, 0}, // 0xF6
  {"ISC", OP_ISC, ADR_ZERO_PAGE_X     , 0, 6, 0}, // 0xF7
  {"SED", OP_SED, ADR_IMPLIED         , 1, 2, 0}, // 0xF8
  {"SBC", OP_SBC, ADR_ABSOLUTE_Y      , 3, 4, 1}, // 0xF9
  {"NOP", OP_NOP, ADR_IMPLIED         , 1, 2, 0}, // 0xFA
  {"ISC", OP_ISC, ADR_ABSOLUTE_Y      , 0, 7, 0}, // 0xFB
  {"NOP", OP_NOP, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0xFC
  {"SBC", OP_SBC, ADR_ABSOLUTE_X      , 3, 4, 1}, // 0xFD
  {"INC", OP_INC, ADR_ABSOLUTE_X      , 3, 7, 0}, // 0xFE
  {"ISC", OP_ISC, ADR_ABSOLUTE_X      , 0, 7, 0}  // 0xFF
};
