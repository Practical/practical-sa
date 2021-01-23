/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <practical/defines.h>
#include <practical/practical.h>
#include <practical/slice.h>

#include <cstring>
#include <iostream>
#include <memory>

using PracticalSemanticAnalyzer::SourceLocation;

namespace Tokenizer {

enum class Tokens {
    ERR, // Error in parsing
    WS, // White space
    COMMENT_LINE_END,
    COMMENT_MULTILINE,
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
    OP_AMPERSAND,
    OP_ARROW,                                   // ->
    OP_ASSIGN,
    OP_ASSIGN_BIT_AND,
    OP_ASSIGN_BIT_OR,
    OP_ASSIGN_BIT_XOR,
    OP_ASSIGN_DIVIDE,
    OP_ASSIGN_LEFT_SHIFT,
    OP_ASSIGN_MINUS,
    OP_ASSIGN_MODULOUS,
    OP_ASSIGN_MULTIPLY,
    OP_ASSIGN_PLUS,
    OP_ASSIGN_RIGHT_SHIFT,
    OP_ASSIGN_RIGHT_SHIFT_LOGICAL,
    OP_MULTIPLY,
    OP_BIT_AND,
    OP_BIT_NOT,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_PTR,
    OP_COLON,
    OP_DIVIDE,
    OP_DOT,
    OP_DOUBLE_COLON,
    OP_PAAMAYIM_NEKUDATAYIM = OP_DOUBLE_COLON,  // Nekudotayim is a misspelling
    OP_EQUALS,
    OP_GREATER_THAN,
    OP_GREATER_THAN_EQ,
    OP_LESS_THAN,
    OP_LESS_THAN_EQ,
    OP_LOGIC_AND,
    OP_LOGIC_NOT,
    OP_TEMPLATE_EXPAND = OP_LOGIC_NOT,
    OP_LOGIC_OR,
    OP_MINUS,
    OP_MINUS_MINUS,
    OP_MODULOUS,
    OP_NOT_EQUALS,
    OP_PLUS,
    OP_PLUS_PLUS,
    OP_SHIFT_LEFT,
    OP_SHIFT_RIGHT,
    OP_RUNON_ERROR,
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
    RESERVED_DECL,
    RESERVED_EXPECT,
    RESERVED_TRUE,
    RESERVED_FALSE,
    RESERVED_IF,
    RESERVED_ELSE,
    RESERVED_REF,
    RESERVED_NULL,
    RESERVED_STRUCT,
};

struct Token {
    String text;
    Tokens token = Tokens::ERR;
    SourceLocation location;
};

class Tokenizer {
private:
    Slice<const char> file;
    SourceLocation location;
    size_t position=0;
    SourceLocation tokenLocation;
    Tokens token;
    Slice<const char> tokenText;

    struct SavedPoint {
        SourceLocation location;
        size_t position;
    };

public:
    Tokenizer(String file) : file(file), location{ .line=1, .col=1 } {
    }

    bool next();

    Token current() const {
        Token ret;

        ret.text = currentTokenText();
        ret.location = currentLocation();
        ret.token = currentToken();

        return ret;
    }

    Tokens currentToken() const {
        return token;
    }

    SourceLocation currentLocation() const {
        return tokenLocation;
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

    void consumeLineComment();
    void consumeNestableComment(SavedPoint startPoint);

    bool nextChar();
    SavedPoint savePosition();
    void restorePosition(SavedPoint position);
};

} // namespace Tokenizer

std::ostream &operator<<(std::ostream &out, Tokenizer::Tokens token);
std::ostream &operator<<(std::ostream &out, const Tokenizer::Token &token);

#endif // TOKENIZER_H
