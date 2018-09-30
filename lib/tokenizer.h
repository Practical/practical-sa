#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <iostream>

#include <slice.h>

class Tokenizer {
    Slice<const char> file;
    size_t line=0, col=0;
    size_t position=0;

public:
    enum class Tokens {
        WS, /// White space
        OPEN_ROUND_BRACKET,
    };

    Tokenizer(Slice<const char> file) : file(file) {
    }

    Tokens token() {
        assert(false); // TODO implement
    }

    void next() {
        assert(false); // TODO implement
    }

    bool done() const {
        assert(false); // TODO implement
    }

    size_t currentLine() const {
        return line;
    }

    size_t currentCol() const {
        return col;
    }

    Slice<const char> currentTokenText() const {
        assert(false); // TODO implement
    }
};

std::ostream &operator<<(std::ostream &out, Tokenizer::Tokens token) {
#define CASE(name) case Tokenizer::Tokens::name: out<<#name; break
    switch(token) {
        CASE(WS);
        CASE(OPEN_ROUND_BRACKET);
    }

    return out;
#undef CASE
}

#endif // TOKENIZER_H
