#include "Parser.h"

#include <stdexcept>
#include <iostream>

#include "Tokenizer.h"

Parser::Parser(std::vector<Token> toks) : toks(toks)
{}

Parser::Parser(const std::string& src) {
    auto tokenizer = Tokenizer(src);
    toks = tokenizer.tokenizeAll();
}

Token Parser::peek(size_t amount) {
    return toks[index + amount];
}

Token Parser::next() {
    if (index >= toks.size()) {
        throw std::runtime_error("Index out of bounds");
    }
    return toks[index++];
}

Token Parser::expect(TokType type) {
    auto tok = next();
    if (tok.type != type) {
        throw std::runtime_error(std::string("Expected ") + tokTypeToString(type)
            + std::string(", got ") + tokenToString(tok));
    }
    return tok;
}

bool Parser::accept(TokType type) {
    if (peek().type == type) {
        next();
        return true;
    }
    return false;
}

Program* Parser::parse() {
    auto tok = peek();
    std::vector<Statement*> stmts;
    std::vector<std::string> reqs;
    while (tok.type == TokType::REQUIRE) {
        next();
        reqs.push_back(expect(TokType::IDENTIFIER).value);
        expect(TokType::SEMICOLON);
        tok = peek();
    }

    while (tok.type != TokType::T_EOF) {
        stmts.push_back(parseStatement());
        tok = peek();
    }
    return new Program(stmts, reqs);
}

Statement* Parser::parseStatement() {
    switch (peek().type) {
        case TokType::IF: return parseIfElse();
        case TokType::LOOP: return parseLoop();
        case TokType::LET: return parseVarDecl();
        case TokType::FN: throw std::runtime_error("Functions not implemented yet!");
        case TokType::LBRACE: return parseBlock();
    }

    Expression* expr = parseExpression();
    expect(TokType::SEMICOLON);
    return new ExprStatement(expr);
}

Statement* Parser::parseIfElse() {
    expect(TokType::IF);
    expect(TokType::LPAREN);
    auto* cond = parseExpression();
    expect(TokType::RPAREN);
    auto* body = parseStatement();
    Statement* elseBody = nullptr;
    if (accept(TokType::ELSE)) {
        elseBody = parseStatement();
    }
    return new IfElse(cond, body, elseBody);
}

Statement* Parser::parseLoop() {
    expect(TokType::LOOP);
    auto* body = parseStatement();
    return new LoopStmt(body);
}

Statement* Parser::parseBlock() {
    expect(TokType::LBRACE);
    std::vector<Statement*> stmts;
    while (peek().type != TokType::RBRACE) {
        stmts.push_back(parseStatement());
    }
    expect(TokType::RBRACE);
    return new BlockStmt(stmts);
}

Statement* Parser::parseVarDecl() {
    expect(TokType::LET);
    std::string id = expect(TokType::IDENTIFIER).value;
    Expression* expr = nullptr;
    if (accept(TokType::ASSIGN)) {
        expr = parseExpression();
    }
    expect(TokType::SEMICOLON);
    return new VarDeclaration(id, expr);
}



Expression* Parser::parseExpression() {
    return parseAssignment();
}

Expression* Parser::parseAssignment() {
    if (peek().type == TokType::IDENTIFIER && peek(1).type == TokType::ASSIGN) {
        std::string id = expect(TokType::IDENTIFIER).value;
        expect(TokType::ASSIGN);
        auto* expr = parseAssignment();
        return new Assignment(id, expr);
    }
    return parseLogicOr();
}

Expression* Parser::parseLogicOr() {
    Expression* node = parseLogicAnd();
    while (accept(TokType::OR)) {
        Expression* rhs = parseLogicAnd();
        node = new BinaryExpr(BinOp::LOR, node, rhs);
    }
    return node;
}

Expression* Parser::parseLogicAnd() {
    Expression* node = parseEquality();
    while (accept(TokType::AND)) {
        Expression* rhs = parseEquality();
        node = new BinaryExpr(BinOp::LAND, node, rhs);
    }
    return node;
}

Expression* Parser::parseEquality() {
    Expression* node = parseComparison();
    while (1) {
        if (accept(TokType::EQUALS)) {
            Expression* rhs = parseComparison();
            node = new BinaryExpr(BinOp::EQUALS, node, rhs);
        } else if (accept(TokType::NEQUALS)) {
            Expression* rhs = parseComparison();
            node = new BinaryExpr(BinOp::NEQUALS, node, rhs);
        } else {
            break;
        }
    }
    return node;
}

Expression* Parser::parseComparison() {
    Expression* node = parseTerm();
    while (1) {
        if (accept(TokType::GREATER)) {
            Expression* rhs = parseTerm();
            node = new BinaryExpr(BinOp::GREATER, node, rhs);
        } else if (accept(TokType::GEQUALS)) {
            Expression* rhs = parseTerm();
            node = new BinaryExpr(BinOp::GEQUALS, node, rhs);
        } else if (accept(TokType::LESS)) {
            Expression* rhs = parseTerm();
            node = new BinaryExpr(BinOp::LESS, node, rhs);
        } else if (accept(TokType::LEQUALS)) {
            Expression* rhs = parseTerm();
            node = new BinaryExpr(BinOp::LEQUALS, node, rhs);
        } else {
            break;
        }
    }
    return node;
}

Expression* Parser::parseTerm() {
    Expression* node = parseFactor();
    while (1) {
        if (accept(TokType::PLUS)) {
            Expression* rhs = parseFactor();
            node = new BinaryExpr(BinOp::ADD, node, rhs);
        } else if (accept(TokType::MINUS)) {
            Expression* rhs = parseFactor();
            node = new BinaryExpr(BinOp::SUB, node, rhs);
        } else {
            break;
        }
    }
    return node;
}

Expression* Parser::parseFactor() {
    Expression* node = parseUnary();
    while (1) {
        if (accept(TokType::MUL)) {
            Expression* rhs = parseUnary();
            node = new BinaryExpr(BinOp::MUL, node, rhs);
        } else if (accept(TokType::DIV)) {
            Expression* rhs = parseUnary();
            node = new BinaryExpr(BinOp::DIV, node, rhs);
        } else if (accept(TokType::MOD)) {
            Expression* rhs = parseUnary();
            node = new BinaryExpr(BinOp::MOD, node, rhs);
        } else {
            break;
        }
    }
    return node;
}

Expression* Parser::parseUnary() {
    if (accept(TokType::MINUS)) {
        Expression* rhs = parseUnary();
        return new BinaryExpr(BinOp::SUB, new NumberExpr(0), rhs);
    }
    return parseCall();
}

Expression* Parser::parseCall() {
    if (peek().type == TokType::IDENTIFIER && peek(1).type == TokType::LPAREN) {
        std::string id = expect(TokType::IDENTIFIER).value;

        if (id == "max") {

        } else if (id == "min") {

        } else if (id == "abs") {

        }

        expect(TokType::LPAREN);
        std::vector<Expression*> args;
        if (!accept(TokType::RPAREN)) {
            args.push_back(parseExpression());
            while (accept(TokType::COMMA)) {
                args.push_back(parseExpression());
            }
            expect(TokType::RPAREN);
        }
        return new CallExpr(id, args);
    }
    return parsePrimary();
}

Expression* Parser::parsePrimary() {
    Token tok = next();
    
    if (tok.type == TokType::NUMBER) {
        return new NumberExpr(std::stoi(tok.value));
    } else if (tok.type == TokType::IDENTIFIER) {
        return new VarExpr(tok.value);
    } else if (tok.type == TokType::LPAREN) {
        Expression* expr = parseExpression();
        expect(TokType::RPAREN);
        return expr;
    } else {
        throw std::runtime_error(std::string("Unexpected token: " + tokenToString(tok)));
    }
}

int main() {
    // Simple blinking program
    std::string program = "require neo_pixel;\nloop {\n\tfill_rgb(255, 0, 0);\n\tdelay(500);\n\tfill_rgb(0, 255, 0);\n\tdelay(500);\n}";
    Parser parser(program);
    Program* prog = parser.parse();
    std::cout << prog->to_string() << std::endl;
}