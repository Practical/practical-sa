#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <iostream>

#include <slice.h>

class tokenizer_error : public std::exception {
    const char *msg;
    size_t line, col;
public:
    tokenizer_error(const char *msg, size_t line, size_t col) : msg(msg), line(line), col(col) {
    }

    const char *what() const noexcept {
        return msg;
    }
};

class Tokenizer {
public:
    enum class Tokens {
        WS, /// White space
        OPEN_ROUND_BRACKET,
    };

private:
    Slice<const char> file;
    size_t line=0, col=0;
    size_t position=0;
    Tokens token;
    Slice<const char> tokenText;

public:
    Tokenizer(Slice<const char> file) : file(file) {
        line = 1;
        col = 1;
        next();
    }

    bool next();

    Tokens currentToken() {
        return token;
    }

    size_t currentLine() const {
        return line;
    }

    size_t currentCol() const {
        return col;
    }

    Slice<const char> currentTokenText() const {
        return tokenText;
    }

private:
    static bool isWS(char chr) {
        return chr==' ' || chr=='\n' || chr=='\r' || chr=='\t';
    }

    void consumeWS();

    void nextChar();
};

std::ostream &operator<<(std::ostream &out, Tokenizer::Tokens token);

#endif // TOKENIZER_H
