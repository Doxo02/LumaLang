#ifndef LUMA_VISITOR_H
#define LUMA_VISITOR_H

#include "../Parser.h"

class Visitor {
    public:
        virtual void visitBinaryExpr(BinaryExpr* binaryExpr) = 0;
        virtual void visitAssignment(Assignment* assignment) = 0;
        virtual void visitCallExpr(CallExpr* callExpr) = 0;
        virtual void visitNumberExpr(NumberExpr* numberExpr) = 0;
        virtual void visitVarExpr(VarExpr* varExpr) = 0;
        virtual void visitExprStatement(ExprStatement* exprStmt) = 0;
        virtual void visitIfElse(IfElse* ifElse) = 0;
        virtual void visitLoopStmt(LoopStmt* loopStmt) = 0;
        virtual void visitBlockStmt(BlockStmt* blockStmt) = 0;
        virtual void visitVarDeclaration(VarDeclaration* varDeclaration) = 0;
        virtual void visitProgram(Program* program) = 0;
};

#endif