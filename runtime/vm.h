#ifndef LUMA_VM_H
#define LUMA_VM_H

#include <stdint.h>
#include <stdbool.h>

/* ------------ Configuration ------------ */
#define STACK_WORDS 256
#define MEM_WORDS 256
#define REG_COUNT 8

#define VM_TRUE 1
#define VM_FALSE 0

/* ------------ VM types & structures ------------ */
typedef int32_t word_t;

typedef struct VM VM;
typedef void (*ExtHandler)(VM *vm, uint8_t subop);

struct VM
{
    word_t regs[REG_COUNT];     // R0..R7
    word_t stack[STACK_WORDS];
    uint8_t sp;                 // stack pointer (wrap-around)
    word_t mem[MEM_WORDS];      // global variable storage
    const uint8_t *code;        // pointer into loaded code section
    uint16_t code_len;          // length of code section
    const uint32_t *consts;     // constant pool pointer
    uint8_t const_count;        // number of constants in pool
    uint16_t pc;                // program counter
    uint8_t flags;              // bitflags defined in file header
    bool halted;
    // delay variables for non-blocking delays
    bool delaying;
    word_t delayAmount;
    uint64_t delayStart;
    ExtHandler ext_table[256];  // registered extension handlers
    int err;                    // Error code (defined below)
};

/* Error codes */
enum
{
    ERR_OK = 0,
    ERR_STACK_UNDERFLOW = 1,
    ERR_STACK_OVERFLOW = 2,
    ERR_DIV_BY_ZERO = 3,
    ERR_BAD_OPCODE = 4,
    ERR_UNKNOWN_EXTENSION = 5,
    ERR_LOAD_FAIL = 6,
};

bool vm_load_program(VM *vm, const uint8_t *code, uint16_t code_len,
                            const uint32_t *consts, uint8_t const_count,
                            bool signed_rel);

void vm_step(VM *vm);   // executes one instruction
void vm_run(VM *vm);    // runs until halted

#endif