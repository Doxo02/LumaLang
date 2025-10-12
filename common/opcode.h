#ifndef OPCODE_H
#define OPCODE_H

enum OpCode
{
    OP_NOOP = 0x00,
    OP_MOVI = 0x01,
    OP_MOV = 0x02,
    OP_LOAD = 0x03,
    OP_STORE = 0x04,
    OP_PUSH = 0x05,
    OP_POP = 0x06,
    OP_LDC = 0x07,

    OP_ADD = 0x10,
    OP_SUB = 0x11,
    OP_MUL = 0x12,
    OP_DIV = 0x13,
    OP_MOD = 0x14,
    OP_ABS = 0x15,
    OP_MAX = 0x16,
    OP_MIN = 0x17,
    OP_AND = 0x18,
    OP_OR = 0x19,
    OP_XOR = 0x1A,
    OP_NOT = 0x1B,

    OP_EQ = 0x20,
    OP_NEQ = 0x21,
    OP_GEQ = 0x22,
    OP_LEQ = 0x23,
    OP_GT = 0x24,
    OP_LT = 0x25,

    OP_JMPA = 0x30,
    OP_JMPR = 0x31,
    OP_JZA = 0x32,
    OP_JZR = 0x33,
    OP_JNZA = 0x34,
    OP_JNZR = 0x35,
    OP_CALLA = 0x36,
    OP_CALLR = 0x37,
    OP_RET = 0x38,

    OP_EXT = 0xE0,

    OP_D_SRGB = 0xD0,
    OP_D_FRGB = 0xD1,
    OP_D_SHOW = 0xD2,
    OP_D_CLR = 0xD3,
    OP_D_NLED = 0xD4,

    OP_DELAY = 0xFD,
    OP_HALT = 0xFF,
};

#endif