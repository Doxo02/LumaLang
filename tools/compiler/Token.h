#ifndef LUMA_TOKEN_H
#define LUMA_TOKEN_H

#include <stdint.h>
#include <string>

enum class TokType {
    // Keywords
    REQUIRE,
    IF,
    ELSE,
    RETURN,
    LOOP,
    LET,
    FN,

    // Operators
    NOT,
    MUL, DIV, MOD,
    PLUS, MINUS,
    GEQUALS, LEQUALS, GREATER, LESS,
    EQUALS, NEQUALS,
    AND, OR,

    // other Symbols
    COMMA, SEMICOLON,
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    ASSIGN,

    IDENTIFIER,
    NUMBER,
    T_EOF
};

struct Token {
    TokType type;
    size_t line;
    size_t col;
    size_t length;
    std::string value = "";
};

static std::string tokenToString(const Token& tok) {
    std::string out = "";
    switch (tok.type) {
        case TokType::REQUIRE: {
            out = "REQUIRE";
        } break;
        case TokType::IF: {
            out = "IF";
        } break;
        case TokType::ELSE: {
            out = "ELSE";
        } break;
        case TokType::RETURN: {
            out = "RETURN";
        } break;
        case TokType::LOOP: {
            out = "LOOP";
        } break;
        case TokType::LET: {
            out = "LET";
        } break;
        case TokType::FN: {
            out = "FN";
        } break;
        case TokType::NOT: {
            out = "NOT";
        } break;
        case TokType::MUL: {
            out = "MUL";
        } break;
        case TokType::DIV: {
            out = "DIV";
        } break;
        case TokType::MOD: {
            out = "MOD";
        } break;
        case TokType::PLUS: {
            out = "PLUS";
        } break;
        case TokType::MINUS: {
            out = "MINUS";
        } break;
        case TokType::GEQUALS: {
            out = "GEQUALS";
        } break;
        case TokType::LEQUALS: {
            out = "LEQUALS";
        } break;
        case TokType::GREATER: {
            out = "GREATER";
        } break;
        case TokType::LESS: {
            out = "LESS";
        } break;
        case TokType::EQUALS: {
            out = "EQUALS";
        } break;
        case TokType::NEQUALS: {
            out = "NEQUALS";
        } break;
        case TokType::AND: {
            out = "AND";
        } break;
        case TokType::OR: {
            out = "OR";
        } break;
        case TokType::COMMA: {
            out = "COMMA";
        } break;
        case TokType::SEMICOLON: {
            out = "SEMICOLON";
        } break;
        case TokType::LPAREN: {
            out = "LPAREN";
        } break;
        case TokType::RPAREN: {
            out = "RPAREN";
        } break;
        case TokType::LBRACE: {
            out = "LBRACE";
        } break;
        case TokType::RBRACE: {
            out = "RBRACE";
        } break;
        case TokType::ASSIGN: {
            out = "ASSIGN";
        } break;
        case TokType::IDENTIFIER: {
            out = "IDENTIFIER";
        } break;
        case TokType::NUMBER: {
            out = "NUMBER";
        } break;
        case TokType::T_EOF: {
            out = "T_EOF";
        } break;
    }
    out.append("(");
    out.append(std::to_string(tok.line));
    out.append(":");
    out.append(std::to_string(tok.col));
    if (tok.length > 1) {
        out.append("-");
        out.append(std::to_string(tok.col + tok.length-1));
    }
    out.append(")");
    if (tok.value != "") {
        out.append(": ");
        out.append(tok.value);
    }
    return out;
}

#endif