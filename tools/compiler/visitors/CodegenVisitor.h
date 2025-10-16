#ifndef LUMA_CODEGEN_VISITOR_H
#define LUMA_CODEGEN_VISITOR_H

#include "Visitor.h"

#include <vector>
#include <stdint.h>
#include <unordered_map>
#include <stdexcept>

class RegAllocater {
    bool used[8] = { false };

    public:
        int alloc() {
            for (int i = 0; i < 8; i++) {
                if (!used[i]) {
                    used[i] = true;
                    return i;
                }
            }
            throw std::runtime_error("Out of registers");
        }

        void free(int r) {
            used[r] = false;
        }
};

class CodegenVisitor : public Visitor {
    std::vector<uint8_t> code;
    std::vector<uint8_t> reqIDs;
    // std::unordered_map<std::string, FunctionDecl*> funcs;
    std::unordered_map<std::string, uint8_t> varMap;
    RegAllocater allocator;

    uint8_t nextVarLoc = 0;
    std::vector<uint8_t> varLocStack;

    public:
        CodegenVisitor();
        
        std::vector<uint8_t> getCode() { return code; }
        std::vector<uint8_t> getLBC();

        virtual int visitBinaryExpr(BinaryExpr* expr) override;
        virtual int visitAssignment(Assignment* expr) override;
        virtual int visitCallExpr(CallExpr* expr) override;
        virtual int visitNumberExpr(NumberExpr* expr) override;
        virtual int visitVarExpr(VarExpr* expr) override;
        virtual void visitExprStatement(ExprStatement* stmt) override;
        virtual void visitIfElse(IfElse* stmt) override;
        virtual void visitLoopStmt(LoopStmt* stmt) override;
        virtual void visitBlockStmt(BlockStmt* stmt) override;
        virtual void visitVarDeclaration(VarDeclaration* stmt) override;
        virtual void visitProgram(Program* program) override;
    
    private:
        void emitu8(uint8_t val);
        void emitu16(uint16_t val);
        void emiti32(int32_t val);

        void emitDestSrc(uint8_t dest, uint8_t src);
};

#endif