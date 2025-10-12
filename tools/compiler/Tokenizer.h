#ifndef LUMA_TOKENIZER_H
#define LUMA_TOKENIZER_H

#include <vector>

#include "Token.h"

class Tokenizer {
    public:
        Tokenizer(const std::string& src);

        std::vector<Token> tokenizeAll();
        Token nextToken();

        std::vector<Token> getTokens();

    private:
        std::string src;
        std::vector<Token> tokens;
        size_t index, line, col;
};

#endif