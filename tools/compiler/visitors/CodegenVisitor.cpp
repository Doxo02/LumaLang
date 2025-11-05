#include "CodegenVisitor.h"
#include "../Parser.h"
#include <opcode.h>
#include "../Extension.h"

CodegenVisitor::CodegenVisitor() 
    : allocator() {}

void CodegenVisitor::emitu8(uint8_t val) {
    code.push_back(val);
}

void CodegenVisitor::emitu16(uint16_t val) {
    code.push_back((uint8_t) (val & 0xFF));
    code.push_back((uint8_t) ((val >> 8) & 0xFF));
}

void CodegenVisitor::emiti32(int32_t val) {
    code.push_back((uint8_t) (val & 0xFF));
    code.push_back((uint8_t) ((val >> 8) & 0xFF));
    code.push_back((uint8_t) ((val >> 16) & 0xFF));
    code.push_back((uint8_t) ((val >> 24) & 0xFF));
}

void CodegenVisitor::emitDestSrc(uint8_t dest, uint8_t src) {
    uint8_t dstsrc = (dest << 4) | src;
}

std::vector<uint8_t> CodegenVisitor::getLBC()
{
    /*
     * Header
     */
    std::vector<uint8_t> out;
    // Magic Number
    out.push_back('L');
    out.push_back('V');
    out.push_back('M');
    out.push_back('1');
    // Bytecode version
    out.push_back(1);
    // Flags
    out.push_back(0);
    // Extension count
    out.push_back(reqIDs.size());
    // Constants count
    out.push_back(0);
    // Code Offset
    uint16_t offset = 16 + reqIDs.size() * 3;
    out.push_back(offset & 0xFF);
    out.push_back((offset >> 8) & 0xFF);
    // Entry Point
    out.push_back(0);
    out.push_back(0);
    // length of code
    out.push_back(code.size() & 0xFF);
    out.push_back((code.size() >> 8) & 0xFF);
    out.push_back((code.size() >> 16) & 0xFF);
    out.push_back((code.size() >> 24) & 0xFF);

    /*
     * Extensions
     */
    for (auto req : reqIDs) {
        out.push_back(req);
        out.push_back(0);
        out.push_back(0);
    }

    /*
     * Code
     */
    out.insert(out.end(), code.begin(), code.end());
    return out;
}

int CodegenVisitor::visitBinaryExpr(BinaryExpr *expr)
{
    int rLhs = expr->lhs->visit(this);
    int rRhs = expr->rhs->visit(this);
    emitu8(OP_MOV);
    emitDestSrc(rLhs, rRhs);
    allocator.free(rRhs);
    return rLhs;
}

int CodegenVisitor::visitAssignment(Assignment *expr) {
    int reg = expr->expr->visit(this);
    auto var = varMap.find(expr->id);
    if (var == varMap.end()) {
        throw std::runtime_error("Tried assigning to undeclared var: " + expr->id);
    }
    emitu8(OP_STORE);
    emitu8(var->second);
    emitu8(reg);
    return reg;
}

int CodegenVisitor::visitCallExpr(CallExpr *expr) {
    if (expr->args.size() > 0) {
        std::vector<int> argRegs;
        for (size_t i = 0; i < expr->args.size(); i++) {
            int reg = expr->args[i]->visit(this);
            argRegs.push_back(reg);
        }

        for (auto& reg : argRegs) {
            allocator.free(reg);
        }

        for (size_t i = 0; i < argRegs.size() && i < 4; i++) {
            if (argRegs[i] != i) {
                emitu8(OP_MOV);
                emitDestSrc((uint8_t) i, argRegs[i]);
            }
        }

        for (size_t i = argRegs.size()-1; i >= 4; i--) {
            emitu8(OP_PUSH);
            emitu8(argRegs[i]);
        }
    }

    if (expr->namesp.size() > 0) {
        auto ext = ExtensionRegistry::instance().get(expr->namesp);

        ExtFunction* fn = ext->getFunction(expr->id);
        if (fn == nullptr) {
            throw std::runtime_error("Unknown extension function: " + expr->namesp + "." + expr->id);
        }

        if (fn->hasReturnValue) {
            if (allocator.is_used(0)) {
                emitu8(OP_PUSH);
                emitu8(0);
            } else {
                allocator.alloc(0);
            }
        }

        emitu8(OP_EXT);
        emitu8(ext->getID());
        emitu8(fn->subOp);
        
        return 0;
    }

    if (expr->id == "delay") {
        emitu8(OP_DELAY);
        emitu8(0);
        return 0;
    }

    throw std::runtime_error("User functions not implemented yet!");
}

int CodegenVisitor::visitNumberExpr(NumberExpr *expr)
{
    int reg = allocator.alloc();
    emitu8(OP_MOVI);
    emitu8(reg);
    emiti32(expr->val);
    return reg;
}

int CodegenVisitor::visitVarExpr(VarExpr *expr) {
    auto var = varMap.find(expr->id);
    if (var == varMap.end()) {
        throw std::runtime_error("Tried accesing undeclared var: " + expr->id);
    }
    int reg = allocator.alloc();
    emitu8(OP_LOAD);
    emitu8(reg);
    emitu8(var->second);
    return reg;
}

void CodegenVisitor::visitExprStatement(ExprStatement *stmt) {
    int reg = stmt->expr->visit(this);
    allocator.free(reg);
}

void CodegenVisitor::visitIfElse(IfElse *stmt) {
    int reg = stmt->cond->visit(this);
    emitu8(OP_JZA);
    uint16_t jmpPos = (uint16_t) code.size();
    emitu16(0);
    stmt->ifBody->visit(this);
    if (stmt->elseBody != nullptr) {
        emitu8(OP_JMPA);
        uint16_t jEndPos = (uint16_t) code.size();
        emitu16(0);

        uint16_t addr = (uint16_t) code.size();
        code[jmpPos] = (uint8_t) (addr & 0xFF);
        code[jmpPos+1] = (uint8_t) ((addr >> 8) & 0xFF);

        stmt->elseBody->visit(this);
        addr = (uint16_t) code.size();
        code[jEndPos] = (uint8_t) (addr & 0xFF);
        code[jEndPos+1] = (uint8_t) ((addr >> 8) & 0xFF);
    } else {
        uint16_t addr = (uint16_t) code.size();
        code[jmpPos] = (uint8_t) (addr & 0xFF);
        code[jmpPos+1] = (uint8_t) ((addr >> 8) & 0xFF);
    }
}

void CodegenVisitor::visitLoopStmt(LoopStmt *stmt) {
    uint16_t loopStart = (uint16_t) code.size();
    stmt->body->visit(this);
    emitu8(OP_JMPA);
    emitu16(loopStart);
}

void CodegenVisitor::visitBlockStmt(BlockStmt *stmt) {
    varLocStack.push_back(nextVarLoc);
    for (auto* s : stmt->stmts) {
        s->visit(this);
    }
    nextVarLoc = varLocStack.back();
    varLocStack.pop_back();
}

void CodegenVisitor::visitVarDeclaration(VarDeclaration *stmt) {
    auto it = varMap.insert({stmt->id, nextVarLoc++});
    int reg;
    if (stmt->expr != nullptr) {
        reg = stmt->expr->visit(this);
    } else {
        reg = allocator.alloc();
    }

    emitu8(OP_STORE);
    emitu8(it.second);
    emitu8(reg);
}

void CodegenVisitor::visitProgram(Program *program) {
    for (auto req : program->reqs) {
        auto ext = ExtensionRegistry::instance().get(req);
        if (ext == nullptr) {
            throw std::runtime_error("Unknown extension: " + req);
        }
        reqIDs.push_back(ext->getID());
    }

    for (auto s : program->stmts) {
        s->visit(this);
    }
}
