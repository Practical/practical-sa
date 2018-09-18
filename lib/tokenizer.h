#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <slice.h>

class Tokenizer {
    Slice<char> file;

public:
    enum Tokens {
        WS, /// White space
    };

    class Token {
    };

    Tokenizer(Slice<char> file) : file(file) {
    }
};

#endif // TOKENIZER_H
