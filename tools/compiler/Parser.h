#ifndef LUMA_PARSER_H
#define LUMA_PARSER_H

#include <string>
#include <vector>

#include "Token.h"
#include "visitors/Visitor.h"

#define IDENT "  "

enum class BinOp {
    ADD, SUB, MUL, DIV, MOD,
    MAX, MIN,
    EQUALS, NEQUALS,
    GREATER, LESS, GEQUALS, LEQUALS,
    LOR, LAND
};

static std::string binOpToString(const BinOp& op) {
    switch (op) {
        case BinOp::ADD: return "ADD";
        case BinOp::SUB: return "SUB";
        case BinOp::MUL: return "MUL";
        case BinOp::DIV: return "DIV";
        case BinOp::MOD: return "MOD";
        case BinOp::MAX: return "MAX";
        case BinOp::MIN: return "MIN";
        case BinOp::EQUALS: return "EQUALS";
        case BinOp::NEQUALS: return "NEQUALS";
        case BinOp::GREATER: return "GREATER";
        case BinOp::LESS: return "LESS";
        case BinOp::GEQUALS: return "GEQUALS";
        case BinOp::LEQUALS: return "LEQUALS";
        default: return "UNKOWN";
    }
}

class ASTNode {
    public:
        virtual std::string to_string(size_t identLevel = 0) {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("ASTNode");
            return ret;
        }

        virtual inline int visit(Visitor* visitor) = 0;
};

class Expression : public ASTNode {
    public:
        virtual std::string to_string(size_t identLevel = 0) override {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("Expression");
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override = 0;
};

class BinaryExpr : public Expression {
    public:
        Expression* lhs;
        Expression* rhs;
        BinOp op;
    
    public:
        BinaryExpr(BinOp op, Expression* lhs, Expression* rhs)
            : op(op), lhs(lhs), rhs(rhs) {}

        virtual std::string to_string(size_t identLevel = 0) override {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("BinaryExpr (");
            ret.append(binOpToString(op));
            ret.append("): {\n");
            ret.append(lhs->to_string(identLevel+1));
            ret.append(",\n");
            ret.append(rhs->to_string(identLevel+1));
            ret.append("\n");
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("}");
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            return visitor->visitBinaryExpr(this);
        }
};

class Assignment : public Expression {
    public:
        std::string id;
        Expression* expr;
    
    public:
        Assignment(const std::string& id, Expression* expr)
            : id(id), expr(expr) {}

        virtual std::string to_string(size_t identLevel = 0) override {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("Assignment (");
            ret.append(id);
            ret.append("): {\n");
            ret.append(expr->to_string(identLevel+1));
            ret.append("\n");
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("}");
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            return visitor->visitAssignment(this);
        }
};

class CallExpr : public Expression {
    public:
        std::string id, namesp;
        std::vector<Expression*> args;

    public:
        CallExpr(const std::string& id, std::vector<Expression*> args, std::string namesp = "")
            : id(id), args(args), namesp(namesp) {}

        virtual std::string to_string(size_t identLevel = 0) {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("CallExpr (");
            if (namesp.size() > 0) {
                ret.append(namesp);
                ret.append(".");
            }
            ret.append(id);
            ret.append(")");
            if (args.size() > 0) {
                ret.append(": {\n");
                for (const auto& arg : args) {
                    ret.append(arg->to_string(identLevel+1));
                    ret.append(",\n");
                }
                for (int i = 0; i < identLevel; i++) ret.append(IDENT);
                ret.append("}");
            }
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            return visitor->visitCallExpr(this);
        }
};

class NumberExpr : public Expression {
    public:
        int32_t val;

    public:
        NumberExpr(int32_t val)
            : val(val) {}

        virtual std::string to_string(size_t identLevel = 0) {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("NumberExpr: ");
            ret.append(std::to_string(val));
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            return visitor->visitNumberExpr(this);
        }
};

class VarExpr : public Expression {
    public:
        std::string id;

    public:
        VarExpr(const std::string& id)
            : id(id) {}

        virtual std::string to_string(size_t identLevel = 0) {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("VarExpr: ");
            ret.append(id);
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            return visitor->visitVarExpr(this);
        }
};

class Statement : public ASTNode {
    public:
        virtual std::string to_string(size_t identLevel = 0) override {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("Statement");
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override = 0;
};

class ExprStatement : public Statement {
    public:
        Expression* expr;

    public:
        ExprStatement(Expression* expr)
            : expr(expr) {}

        virtual std::string to_string(size_t identLevel = 0) override {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("ExprStatement:\n");
            ret.append(expr->to_string(identLevel+1));
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            visitor->visitExprStatement(this);
            return 0;
        }
};

class IfElse : public Statement {
    public:
        Expression* cond;
        Statement* ifBody;
        Statement* elseBody;

    public:
        IfElse(Expression* cond, Statement* ifBody, Statement* elseBody = nullptr)
            : cond(cond), ifBody(ifBody), elseBody(elseBody) {}

        virtual std::string to_string(size_t identLevel = 0) override {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("If: {\n");
            for (int i = 0; i < identLevel+1; i++) ret.append(IDENT);
            ret.append("condition:\n");
            ret.append(cond->to_string(identLevel+2));
            for (int i = 0; i < identLevel+1; i++) ret.append(IDENT);
            ret.append("body:\n");
            ret.append(ifBody->to_string(identLevel+2));
            if (elseBody != nullptr) {
                ret.append("\n");
                for (int i = 0; i < identLevel+1; i++) ret.append(IDENT);
                ret.append("else body:\n");
                ret.append(elseBody->to_string(identLevel+1));
            }
            ret.append("\n");
            for (int i = 0; i < identLevel+1; i++) ret.append(IDENT);
            ret.append("}");
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            visitor->visitIfElse(this);
            return 0;
        }
};

class LoopStmt : public Statement {
    public:
        Statement* body;

    public:
        LoopStmt(Statement* body)
            : body(body) {}

        virtual std::string to_string(size_t identLevel = 0) override {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("Loop:\n");
            ret.append(body->to_string(identLevel+1));
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            visitor->visitLoopStmt(this);
            return 0;
        }
};

class BlockStmt : public Statement {
    public:
        std::vector<Statement*> stmts;

    public:
        BlockStmt(std::vector<Statement*> stmts)
            : stmts(stmts) {}

        virtual std::string to_string(size_t identLevel = 0) override {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("Block: {\n");
            for (const auto& stmt : stmts) {
                ret.append(stmt->to_string(identLevel+1));
                ret.append(",\n");
            }
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("}");
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            visitor->visitBlockStmt(this);
            return 0;
        }
};

class VarDeclaration : public Statement {
    public:
        std::string id;
        Expression* expr;

    public:
        VarDeclaration(const std::string& id, Expression* expr = nullptr)
            : id(id), expr(expr) {}

        virtual std::string to_string(size_t identLevel = 0) override {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("VarDeclaration (");
            ret.append(id);
            ret.append(")");
            if (expr != nullptr) {
                ret.append(":\n");
                ret.append(expr->to_string(identLevel+1));
            }
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            visitor->visitVarDeclaration(this);
            return 0;
        }
};

// TODO: Function

class Program : public ASTNode {
    public:
        std::vector<Statement*> stmts;
        std::vector<std::string> reqs;

    public:
        Program(std::vector<Statement*> stmts, std::vector<std::string> reqs)
            : stmts(stmts), reqs(reqs) {}

        virtual std::string to_string(size_t identLevel = 0) override {
            std::string ret = "";
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("Program: {\n");
            for (int i = 0; i < identLevel+1; i++) ret.append(IDENT);
            ret.append("Requires: [\n");
            for (auto req : reqs) {
                for (int i = 0; i < identLevel+2; i++) ret.append(IDENT);
                ret.append(req);
                ret.append(",\n");
            }
            for (int i = 0; i < identLevel+1; i++) ret.append(IDENT);
            ret.append("]\n");
            for (int i = 0; i < identLevel+1; i++) ret.append(IDENT);
            ret.append("Statements: [\n");
            for (auto stmt : stmts) {
                ret.append(stmt->to_string(identLevel+2));
                ret.append(",\n");
            }
            for (int i = 0; i < identLevel+1; i++) ret.append(IDENT);
            ret.append("]\n");
            for (int i = 0; i < identLevel; i++) ret.append(IDENT);
            ret.append("}");
            return ret;
        }

        virtual inline int visit(Visitor* visitor) override {
            visitor->visitProgram(this);
            return 0;
        }
};

class Parser {
    std::vector<Statement*> stmts;
    std::vector<Token> toks;

    size_t index = 0;

    public:
        Parser(std::vector<Token> toks);
        Parser(const std::string& src);

        Program* parse();

    private:
        Token peek(size_t amount = 0);
        Token next();
        Token expect(TokType type);
        bool accept(TokType type);

        Statement* parseStatement();
        Statement* parseIfElse();
        Statement* parseLoop();
        Statement* parseBlock();
        Statement* parseVarDecl();
        // Statement* parseFnDecl();

        Expression* parseExpression();
        Expression* parseAssignment();
        Expression* parseLogicOr();
        Expression* parseLogicAnd();
        Expression* parseEquality();
        Expression* parseComparison();
        Expression* parseTerm();
        Expression* parseFactor();
        Expression* parseUnary();
        Expression* parseCall();
        Expression* parsePrimary();
};


#endif