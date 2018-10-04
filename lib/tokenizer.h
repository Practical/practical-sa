#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <cstring>
#include <iostream>
#include <memory>

#include "slice.h"

class tokenizer_error : public std::exception {
    size_t line, col;
    std::unique_ptr<char> msg;
public:
    tokenizer_error(const char *msg, size_t line, size_t col) : line(line), col(col) {
        size_t buffsize = strlen(msg) + 100;
        this->msg = std::unique_ptr<char>(new char[buffsize]);
        snprintf(this->msg.get(), buffsize, "%s at %lu:%lu", msg, line, col);
    }

    const char *what() const noexcept {
        return msg.get();
    }
};

class Tokenizer {
public:
    enum class Tokens {
        WS, /// White space
        SEMICOLON,
        // Bracket types
        BRACKET_ROUND_OPEN,
        BRACKET_ROUND_CLOSE,
        BRACKET_SQUARE_OPEN,
        BRACKET_SQUARE_CLOSE,
        BRACKET_CURLY_OPEN,
        BRACKET_CURLY_CLOSE,
        // Operators
        // Literals
        LITERAL_INT_2,
        LITERAL_INT_8,
        LITERAL_INT_10,
        LITERAL_INT_16,
        LITERAL_FP,
        LITERAL_STRING,
        // Identifier like
        IDENTIFIER,
        RESERVED_DEF,
    };

private:
    Slice<const char> file;
    size_t line=0, col=0;
    size_t position=0;
    size_t tokenLine=0, tokenCol=0;
    Tokens token;
    Slice<const char> tokenText;

public:
    Tokenizer(Slice<const char> file) : file(file), line(1), col(1) {
    }

    bool next();

    Tokens currentToken() {
        return token;
    }

    size_t currentLine() const {
        return tokenLine;
    }

    size_t currentCol() const {
        return tokenCol;
    }

    Slice<const char> currentTokenText() const {
        return tokenText;
    }

private:
    // XXX all of the is* functions here are ASCII
    static bool isWS(char chr) {
        return chr==' ' || chr=='\n' || chr=='\r' || chr=='\t';
    }

    static bool isDigit(char chr) {
        return chr>='0' && chr<='9';
    }

    // Whether chr is an alpha character suitable for a literal
    static bool isIdentifierAlpha(char chr) {
        return (chr>='A' && chr<='Z') || (chr>='a' && chr<='z') || chr=='_';
    }

    void consumeWS();
    void consumeStringLiteral();
    void consumeNumericLiteral();
    void consumeIdentifier();

    bool nextChar();
};

std::ostream &operator<<(std::ostream &out, Tokenizer::Tokens token);

#endif // TOKENIZER_H
