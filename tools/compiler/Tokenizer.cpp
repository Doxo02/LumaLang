#include "Tokenizer.h"

#include <iostream>

Tokenizer::Tokenizer(const std::string &src) : src(std::move(src))
{
    line = 0;
    col = 0;
    index = 0;
}

std::vector<Token> Tokenizer::tokenizeAll()
{
    while (nextToken().type != TokType::T_EOF);
    return tokens;
}

Token Tokenizer::nextToken()
{
    char cur = src[index];
    while (index < src.length() && (cur == ' ' || cur == '\t' || cur == '\n')) {
        if (cur == '\n') {
            line++;
            col = 0;
        } else {
            col++;
        }
        cur = src[++index];
    }

    if (index >= src.length()) {
        Token t = Token{TokType::T_EOF, line, col, 0};
        tokens.push_back(t);
        return t;
    }

    if (isalpha(cur) || cur == '_') {
        std::string buf = "";
        buf += cur;
        cur = src[++index];
        while (isalnum(cur) || cur == '_') {
            buf += cur;
            cur = src[++index];
        }

        Token t = Token{TokType::T_EOF, 0, 0, 0};
        if (buf == "require") {
            t = Token{TokType::REQUIRE, line, col, buf.length()};
        } else if (buf == "if") {
            t = Token{TokType::IF, line, col, buf.length()};
        } else if (buf == "else") {
            t = Token{TokType::ELSE, line, col, buf.length()};
        } else if (buf == "return") {
            t = Token{TokType::RETURN, line, col, buf.length()};
        } else if (buf == "loop") {
            t = Token{TokType::LOOP, line, col, buf.length()};
        } else if (buf == "let") {
            t = Token{TokType::LET, line, col, buf.length()};
        } else if (buf == "fn") {
            t = Token{TokType::FN, line, col, buf.length()};
        } else if (buf == "and") {
            t = Token{TokType::AND, line, col, buf.length()};
        } else if (buf == "or") {
            t = Token{TokType::OR, line, col, buf.length()};
        }

        if (t.type == TokType::T_EOF) {
            t = Token{TokType::IDENTIFIER, line, col, buf.length(), buf};
        }
        tokens.push_back(t);
        col += buf.length();
        return t;
    }

    if (isdigit(cur)) {
        std::string buf = "";
        buf += cur;
        cur = src[++index];
        while (isdigit(cur)) {
            buf += cur;
            cur = src[++index];
        }
        Token t = Token{TokType::NUMBER, line, col, buf.length(), buf};
        tokens.push_back(t);
        col += buf.length();
        return t;
    }

    Token ret;
    switch (cur) {
        case '+': {
            ret = Token{TokType::PLUS, line, col, 1};
        } break;
        case '-': {
            ret = Token{TokType::MINUS, line, col, 1};
        } break;
        case '*': {
            ret = Token{TokType::MUL, line, col, 1};
        } break;
        case '/': {
            ret = Token{TokType::DIV, line, col, 1};
        } break;
        case '%': {
            ret = Token{TokType::MOD, line, col, 1};
        } break;
        case ',': {
            ret = Token{TokType::COMMA, line, col, 1};
        } break;
        case ';': {
            ret = Token{TokType::SEMICOLON, line, col, 1};
        } break;
        case '(': {
            ret = Token{TokType::LPAREN, line, col, 1};
        } break;
        case ')': {
            ret = Token{TokType::RPAREN, line, col, 1};
        } break;
        case '{': {
            ret = Token{TokType::LBRACE, line, col, 1};
        } break;
        case '}': {
            ret = Token{TokType::RBRACE, line, col, 1};
        } break;
        case '!': {
            if (src[index] == '=') {
                index++;
                col++;
                ret = Token{TokType::NEQUALS, line, col, 2};
            } else {
                ret = Token{TokType::NOT, line, col, 1};
            }
        } break;
        case '>': {
            if (src[index] == '=') {
                index++;
                col++;
                ret = Token{TokType::GEQUALS, line, col, 2};
            } else {
                ret = Token{TokType::GREATER, line, col, 1};
            }
        } break;
        case '<': {
            if (src[index] == '=') {
                index++;
                col++;
                ret = Token{TokType::LEQUALS, line, col, 2};
            } else {
                ret = Token{TokType::LESS, line, col, 1};
            }
        } break;
        case '=': {
            if (src[index] == '=') {
                index++;
                col++;
                ret = Token{TokType::EQUALS, line, col, 2};
            } else {
                ret = Token{TokType::ASSIGN, line, col, 1};
            }
        } break;
        default: {
            std::cerr << "idk" << std::endl;
            exit(1);
        }
    }

    col++;
    index++;
    tokens.push_back(ret);
    return ret;
}

std::vector<Token> Tokenizer::getTokens()
{
    Token t = nextToken();
    while (t.type != TokType::T_EOF) {
        t = nextToken();
    }

    return tokens;
}