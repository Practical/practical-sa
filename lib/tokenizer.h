#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <cstring>
#include <iostream>
#include <memory>

#include "defines.h"
#include "slice.h"

class tokenizer_error : public compile_error {
public:
    tokenizer_error(const char *msg, size_t line, size_t col) : compile_error(msg, line, col) {
    }
};

class Tokenizer {
public:
    enum class Tokens {
        ERR, // Error in parsing
        WS, // White space
        COMMENT,
        // Syntax separators
        SEMICOLON,
        COMMA,
        // Bracket types
        BRACKET_ROUND_OPEN,
        BRACKET_ROUND_CLOSE,
        BRACKET_SQUARE_OPEN,
        BRACKET_SQUARE_CLOSE,
        BRACKET_CURLY_OPEN,
        BRACKET_CURLY_CLOSE,
        // Operators
        OP_ASTERISK,
        OP_ARROW,                       // ->
        OP_LOGIC_NOT,
        OP_LOGIC_AND,
        OP_LOGIC_OR,
        OP_MODULOUS,
        OP_MINUS,
        OP_MINUS_MINUS,
        OP_PLUS,
        OP_PLUS_PLUS,
        OP_BIT_AND,
        OP_BIT_OR,
        OP_BIT_NOT,
        OP_ASSIGN,
        OP_ASSIGN_MINUS,
        OP_ASSIGN_PLUS,
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

    struct Token {
        String text;
        Tokens token = Tokens::ERR;
        size_t line=0, col=0;
    };

private:
    Slice<const char> file;
    size_t line=0, col=0;
    size_t position=0;
    size_t tokenLine=0, tokenCol=0;
    Tokens token;
    Slice<const char> tokenText;

public:
    Tokenizer(String file) : file(file), line(1), col(1) {
    }

    bool next();

    Token current() const {
        Token ret;

        ret.text = currentTokenText();
        ret.line = currentLine();
        ret.col = currentCol();
        ret.token = currentToken();

        return ret;
    }

    Tokens currentToken() const {
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

    static std::vector<Token> tokenize(String source);

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
    void consumeOp();
    void consumeStringLiteral();
    void consumeNumericLiteral();
    void consumeIdentifier();

    bool nextChar();
};

std::ostream &operator<<(std::ostream &out, Tokenizer::Tokens token);

#endif // TOKENIZER_H
