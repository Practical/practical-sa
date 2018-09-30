#include <tokenizer.h>

bool Tokenizer::next() {
    if( file.length()==position ) {
        // We've reached our EOF
        return false;
    }

    size_t startLine = currentLine();
    size_t startCol = currentCol();
    size_t tokenStart = position;
    char currentChar = file[tokenStart];
    if(isWS(currentChar)) {
        consumeWS();
    } else {
        throw tokenizer_error("Invalid character encountered", startLine, startCol);
    }
    tokenText = file.subslice(tokenStart, position);

    return true;
}

void Tokenizer::consumeWS() {
    token = Tokens::WS;
    while( position<file.length() && isWS(file[position]) ) {
        if( file[position]=='\n' ) {
            col=0;
            line++;
        }
        nextChar();
    }
}

void Tokenizer::nextChar() {
    position++;
    col++;
}

std::ostream &operator<<(std::ostream &out, Tokenizer::Tokens token) {
#define CASE(name) case Tokenizer::Tokens::name: out<<#name; break
    switch(token) {
        CASE(WS);
        CASE(OPEN_ROUND_BRACKET);
    }

    return out;
#undef CASE
}
