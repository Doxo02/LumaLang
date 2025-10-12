#include <string.h>

#include "vm.h"
#include "../common/opcode.h"

/* ------------ Helper fetch functions ------------ */
static bool vm_fetch_u8(VM* vm, uint8_t* out) {
    if (vm->pc >= vm->code_len) return false;
    *out = vm->code[vm->pc++];
    return true;
}

static bool vm_fetch_i8(VM* vm, int8_t* out) {
    if (vm->pc >= vm->code_len) return false;
    *out = (int8_t) vm->code[vm->pc++];
    return true;
}

static bool vm_fetch_u16(VM* vm, uint16_t* out) {
    uint8_t lo, hi;
    if (!vm_fetch_u8(vm, &lo)) return false;
    if (!vm_fetch_u8(vm, &hi)) return false;
    *out = (uint16_t) lo | ((uint16_t) hi << 8);
    return true;
}

static bool vm_fetch_i32(VM* vm, int32_t* out) {
    uint8_t b0, b1, b2, b3;
    if (!vm_fetch_u8(vm, &b0)) return false;
    if (!vm_fetch_u8(vm, &b1)) return false;
    if (!vm_fetch_u8(vm, &b2)) return false;
    if (!vm_fetch_u8(vm, &b3)) return false;
    *out = (int32_t)b0 | ((int32_t) b1 << 8) | ((int32_t) b2 << 16) | ((int32_t) b3 << 24);
    return true;
}

static uint8_t op_dst(uint8_t b) { return (b >> 4) & 0x0F; }
static uint8_t op_src(uint8_t b) { return b & 0x0F; }

/* ------------ Stack operations ------------ */
static int vm_push(VM* vm, word_t in) {
    if (vm->sp == STACK_WORDS-1) return ERR_STACK_OVERFLOW;
    vm->stack[++vm->sp] = in;
    return ERR_OK;
}

static int vm_pop(VM* vm, word_t* out) {
    if (vm->sp == 0) return ERR_STACK_UNDERFLOW;
    *out = vm->stack[vm->sp--];
    return ERR_OK;
}

/* ------------ Jump Helper ------------ */
void op_jmpa(VM* vm) {
    uint16_t abs;
    if (!vm_fetch_u16(vm, &abs)) {
        vm->err = ERR_BAD_OPCODE;
        vm->halted = true;
        return;
    }
    if (abs < vm->code_len) {
        vm->pc = abs;
    } else {
        vm->err = ERR_BAD_OPCODE;
        vm->halted = true;
    }
}

void op_jmpr(VM* vm) {
    int8_t rel;
    if (!vm_fetch_i8(vm, &rel)) {
        vm->err = ERR_BAD_OPCODE;
        vm->halted = true;
        return;
    }
    uint16_t hi = vm->pc & 0xFF00;
    uint16_t addr = ((uint16_t) (((int8_t)vm->pc) + rel)) | hi;
    if (addr < vm->code_len) {
        vm->pc = addr;
    } else {
        vm->err = ERR_BAD_OPCODE;
        vm->halted = true;
    }
}

/* ------------ Extension Helper ------------ */
static void ext_dispatch(VM* vm, uint8_t extID, uint8_t subop) {
    ExtHandler h = vm->ext_table[extID];
    if (!h) {
        vm->err = ERR_UNKNOWN_EXTENSION;
        vm->halted = true;
        return;
    }
    h(vm, subop);
}

static void neopixel_ext(VM *vm, uint8_t subop)
{
    printf("neopixel_ext called subop=%02X R0=%ld R1=%ld R2=%ld R3=%ld\n",
           subop, (long)vm->regs[0], (long)vm->regs[1], (long)vm->regs[2], (long)vm->regs[3]);
}

static bool vm_load_program(VM *vm, const uint8_t *code, uint16_t code_len,
                            const uint32_t *consts, uint8_t const_count,
                            bool signed_rel)
{
    if (!vm)
        return false;
    vm->code = code;
    vm->code_len = code_len;
    vm->consts = consts;
    vm->const_count = const_count;
    vm->pc = 0;
    vm->sp = 0; // empty stack
    vm->flags = 0;
    vm->halted = false;
    vm->delaying = false;
    vm->delayAmount = 0;
    vm->delayStart = 0;
    vm->err = ERR_OK;
    // zero regs/mem
    memset(vm->regs, 0, sizeof(vm->regs));
    memset(vm->mem, 0, sizeof(vm->mem));
    for (int i = 0; i < 256; i++)
        vm->ext_table[i] = NULL;
    vm->ext_table[0x01] = neopixel_ext;
    return true;
}

void vm_step(VM* vm) {
    if (vm->halted) return;
    uint8_t op;
    if (!vm_fetch_u8(vm, &op)) {
        vm->err = ERR_BAD_OPCODE;
        vm->halted = true;
        return;
    }

    switch (op) {
        // Data movement
        case OP_NOOP: break;
        case OP_MOVI: {
            uint8_t dst;
            int32_t imm;
            if (!vm_fetch_u8(vm, &dst) || !vm_fetch_i32(vm, &imm)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (dst < REG_COUNT) {
                vm->regs[dst] = imm;
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_MOV: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                vm->regs[dst] = vm->regs[src];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_LOAD: {
            uint8_t dst, addr;
            if (!vm_fetch_u8(vm, &dst) || !vm_fetch_u8(vm, &addr)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (dst < REG_COUNT && addr < MEM_WORDS) {
                vm->regs[dst] = vm->mem[addr];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted;
            }
            break;
        }
        case OP_STORE: {
            uint8_t addr, src;
            if (!vm_fetch_u8(vm, &addr) || vm_fetch_u8(vm, &src)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (addr < MEM_WORDS && src < REG_COUNT) {
                vm->mem[addr] = vm->regs[src];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_PUSH: {
            uint8_t src;
            if (!vm_fetch_u8(vm, &src)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (src < REG_COUNT) {
                vm->err = vm_push(vm, vm->regs[src]);
                if (vm->err) vm->halted = true; 
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_POP: {
            uint8_t dst;
            if (!vm_fetch_u8(vm, &dst)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (dst < REG_COUNT) {
                vm->err = vm_pop(vm, &vm->regs[dst]);
                if (vm->err) vm->halted = true;
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_LDC: {
            uint8_t dst, idx;
            if (!vm_fetch_u8(vm, &dst) || vm_fetch_u8(vm, &idx)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (dst < REG_COUNT && idx < vm->const_count) {
                vm->regs[dst] = vm->consts[idx];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        // Arithmetic
        case OP_ADD: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                vm->regs[dst] += vm->regs[src];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_SUB: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                vm->regs[dst] -= vm->regs[src];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_MUL: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                vm->regs[dst] *= vm->regs[src];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_DIV: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                if (vm->regs[src] == 0) {
                    vm->err = ERR_DIV_BY_ZERO;
                    vm->halted = true;
                } else{
                    vm->regs[dst] /= vm->regs[src];
                }
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_MOD: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                if (vm->regs[src] == 0) {
                    vm->err = ERR_DIV_BY_ZERO;
                    vm->halted = true;
                } else{
                    vm->regs[dst] %= vm->regs[src];
                }
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_ABS: {
            uint8_t dst;
            if (!vm_fetch_u8(vm, &dst)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (dst < REG_COUNT) {
                word_t v = vm->regs[dst];
                vm->regs[dst] = v < 0 ? -v : v;
            }
        }
        case OP_MAX: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                if (vm->regs[src] > vm->regs[dst]) vm->regs[dst] = vm->regs[src];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_MIN: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                if (vm->regs[src] < vm->regs[dst]) vm->regs[dst] = vm->regs[src];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_AND: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                vm->regs[dst] &= vm->regs[src];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_OR: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                vm->regs[dst] |= vm->regs[src];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_XOR: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                vm->regs[dst] ^= vm->regs[src];
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_NOT: {
            uint8_t dst;
            if (!vm_fetch_u8(vm, &dst)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (dst < REG_COUNT) {
                word_t v = vm->regs[dst];
                vm->regs[dst] = ~v;
            }
        }
        // Comparisons
        case OP_EQ: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                if (vm->regs[dst] == vm->regs[src]) vm->regs[dst] = VM_TRUE;
                else vm->regs[dst] = VM_FALSE;
            }
            break;
        }
        case OP_NEQ: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                if (vm->regs[dst] != vm->regs[src]) vm->regs[dst] = VM_TRUE;
                else vm->regs[dst] = VM_FALSE;
            }
            break;
        }
        case OP_GEQ: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                if (vm->regs[dst] >= vm->regs[src]) vm->regs[dst] = VM_TRUE;
                else vm->regs[dst] = VM_FALSE;
            }
            break;
        }
        case OP_LEQ: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                if (vm->regs[dst] <= vm->regs[src]) vm->regs[dst] = VM_TRUE;
                else vm->regs[dst] = VM_FALSE;
            }
            break;
        }
        case OP_GT: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                if (vm->regs[dst] > vm->regs[src]) vm->regs[dst] = VM_TRUE;
                else vm->regs[dst] = VM_FALSE;
            }
            break;
        }
        case OP_LT: {
            uint8_t dstsrc;
            if (!vm_fetch_u8(vm, &dstsrc)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            uint8_t dst = op_dst(dstsrc);
            uint8_t src = op_src(dstsrc);
            if (dst < REG_COUNT && src < REG_COUNT) {
                if (vm->regs[dst] < vm->regs[src]) vm->regs[dst] = VM_TRUE;
                else vm->regs[dst] = VM_FALSE;
            }
            break;
        }
        // Control flow
        case OP_JMPA: {
            op_jmpa(vm);
            break;
        }
        case OP_JMPR: {
            op_jmpr(vm);
            break;
        }
        case OP_JZA: {
            uint8_t cond;
            if (!vm_fetch_u8(vm, &cond)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (cond < REG_COUNT) {
                if (vm->regs[cond] == 0) op_jmpa(vm);
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_JZR: {
            uint8_t cond;
            if (!vm_fetch_u8(vm, &cond)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (cond < REG_COUNT) {
                if (vm->regs[cond] == 0) op_jmpr(vm);
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_JNZA: {
            uint8_t cond;
            if (!vm_fetch_u8(vm, &cond)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (cond < REG_COUNT) {
                if (vm->regs[cond] != 0) op_jmpa(vm);
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_JNZR: {
            uint8_t cond;
            if (!vm_fetch_u8(vm, &cond)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (cond < REG_COUNT) {
                if (vm->regs[cond] != 0) op_jmpr(vm);
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_CALLA: {
            uint16_t abs;
            if (!vm_fetch_u16(vm, &abs)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            vm->err = vm_push(vm, vm->pc);
            if (vm->err) {
                vm->halted = true;
                break;
            }
            if (abs < vm->code_len) {
                vm->pc = abs;
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_CALLR: {
            int8_t rel;
            if (!vm_fetch_i8(vm, &rel)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            vm->err = vm_push(vm, vm->pc);
            if (vm->err) {
                vm->halted = true;
                break;
            }
            uint16_t hi = vm->pc & 0xFF00;
            uint16_t addr = ((uint16_t) (((int8_t)vm->pc) + rel)) | hi;
            if (addr < vm->code_len) {
                vm->pc = addr;
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        case OP_RET: {
            uint16_t addr;
            vm->err = vm_pop(vm, &addr);
            if (vm->err) {
                vm->halted = true;
                break;
            }
            if (addr < vm->code_len) {
                vm->pc = addr;
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
            }
            break;
        }
        // Extensions
        case OP_D_SRGB: {
            ext_dispatch(vm, 0x01, 0x00);
            break;
        }
        case OP_D_FRGB: {
            ext_dispatch(vm, 0x01, 0x01);
            break;
        }
        case OP_D_SHOW: {
            ext_dispatch(vm, 0x01, 0x02);
            break;
        }
        case OP_D_CLR: {
            ext_dispatch(vm, 0x01, 0x03);
            break;
        }
        case OP_D_NLED: {
            ext_dispatch(vm, 0x01, 0x04);
            break;
        }
        case OP_EXT: {
            uint8_t extID, subOp;
            if (!vm_fetch_u8(vm, &extID) || !vm_fetch_u8(vm, &subOp)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            ext_dispatch(vm, extID, subOp);
            break;
        }
        // System
        case OP_HALT: {
            vm->halted = true;
            break;
        }
        case OP_DELAY: {
            uint8_t Rdelay;
            if (!vm_fetch_u8(vm, &Rdelay)) {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            if (Rdelay < REG_COUNT) {
                vm->delaying = true;
                vm->delayAmount = vm->regs[Rdelay];
                vm->delayStart = 0;
            } else {
                vm->err = ERR_BAD_OPCODE;
                vm->halted = true;
                break;
            }
            break;
        }
    }
}

const uint8_t prog[] = {
    OP_NOOP
};