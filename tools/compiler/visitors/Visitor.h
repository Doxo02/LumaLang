#ifndef LUMA_VISITOR_H
#define LUMA_VISITOR_H

class BinaryExpr;
class Assignment;
class CallExpr;
class NumberExpr;
class VarExpr;
class ExprStatement;
class IfElse;
class LoopStmt;
class BlockStmt;
class VarDeclaration;
class Program;

class Visitor {
    public:
        virtual int visitBinaryExpr(BinaryExpr* binaryExpr) = 0;
        virtual int visitAssignment(Assignment* assignment) = 0;
        virtual int visitCallExpr(CallExpr* callExpr) = 0;
        virtual int visitNumberExpr(NumberExpr* numberExpr) = 0;
        virtual int visitVarExpr(VarExpr* varExpr) = 0;
        virtual void visitExprStatement(ExprStatement* exprStmt) = 0;
        virtual void visitIfElse(IfElse* ifElse) = 0;
        virtual void visitLoopStmt(LoopStmt* loopStmt) = 0;
        virtual void visitBlockStmt(BlockStmt* blockStmt) = 0;
        virtual void visitVarDeclaration(VarDeclaration* varDeclaration) = 0;
        virtual void visitProgram(Program* program) = 0;
};

#endif