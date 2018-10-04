#include <regex>
#include <string>
#include <unordered_map>

#include <tokenizer.h>

bool Tokenizer::next() {
    if( file.length()==position ) {
        // We've reached our EOF
        tokenLine = line;
        tokenCol = col;
        return false;
    }

    size_t startLine = line;
    size_t startCol = col;
    size_t tokenStart = position;
    char currentChar = file[tokenStart];
    if(isWS(currentChar)) {
        consumeWS();
    } else if(currentChar==';') {
        nextChar();
        token = Tokens::SEMICOLON;
    } else if(currentChar=='(') {
        nextChar();
        token = Tokens::BRACKET_ROUND_OPEN;
    } else if(currentChar==')') {
        nextChar();
        token = Tokens::BRACKET_ROUND_CLOSE;
    } else if(currentChar=='[') {
        nextChar();
        token = Tokens::BRACKET_SQUARE_OPEN;
    } else if(currentChar==']') {
        nextChar();
        token = Tokens::BRACKET_SQUARE_CLOSE;
    } else if(currentChar=='{') {
        nextChar();
        token = Tokens::BRACKET_CURLY_OPEN;
    } else if(currentChar=='}') {
        nextChar();
        token = Tokens::BRACKET_CURLY_CLOSE;
    } else if(currentChar=='"') {
        consumeStringLiteral();
    } else if(isDigit(currentChar)) {
        consumeNumericLiteral();
    } else if(isIdentifierAlpha(currentChar)) {
        consumeIdentifier();
    } else {
        throw tokenizer_error("Invalid character encountered", startLine, startCol);
    }
    tokenText = file.subslice(tokenStart, position);
    tokenLine = startLine;
    tokenCol = startCol;

    return true;
}

void Tokenizer::consumeWS() {
    token = Tokens::WS;
    while( nextChar() && isWS(file[position]) )
        ;
}

void Tokenizer::consumeStringLiteral() {
    bool moreData = true;
    while( moreData && (moreData=nextChar()) && file[position]!='"' ) {
        if( file[position]=='\\' ) {
            moreData = nextChar();
        } else if( file[position]=='\n' ) {
            throw tokenizer_error("Naked new line in string literal", line, col);
        }
    }

    if( file[position]=='"' ) {
        // Consume the terminating quote itself
        nextChar();
        token = Tokens::LITERAL_STRING;
    } else
        throw tokenizer_error("Unterminated string", line, col);

}

void Tokenizer::consumeNumericLiteral() {
    size_t startLine = line;
    size_t startCol = col;
    size_t startPos = position;
    // Consume all relevant characters, whether legal in an integer literal or not. Sort everything else later.
    while( nextChar() && (isDigit(file[position]) || isIdentifierAlpha(file[position])) ) {
    }

    static std::regex re10("^([0-9][0-9_]*)");
    static std::regex reFp("^(\\.[0-9]+)|(([0-9]+)(\\.([0-9]*))?)([eE][-+]?[0-9_]+)?");
    static std::regex re16("^0[xX]([0-9a-fA-F_]+)");
    static std::regex re2("^0[bB]([01_]+)");
    static std::regex re8("^0[o]([01_]+)");

    const char *start = &file[startPos], *end = &file[position];
    if( std::regex_match( start, end, re10 ) ) {
        token = Tokens::LITERAL_INT_10;
    } else if( std::regex_match( start, end, reFp ) ) {
        token = Tokens::LITERAL_FP;
    } else if( std::regex_match( start, end, re16 ) ) {
        token = Tokens::LITERAL_INT_16;
    } else if( std::regex_match( start, end, re2 ) ) {
        token = Tokens::LITERAL_INT_2;
    } else if( std::regex_match( start, end, re8 ) ) {
        token = Tokens::LITERAL_INT_8;
    } else {
        throw tokenizer_error("Invalid numeric literal", startLine, startCol);
    }
}

static const std::unordered_map<std::string, Tokenizer::Tokens> reservedWords { { "def", Tokenizer::Tokens::RESERVED_DEF } };

void Tokenizer::consumeIdentifier() {
    std::string id;

    id += file[position];
    while( nextChar() && (isIdentifierAlpha(file[position]) || isDigit(file[position])) ) {
        id += file[position];
    }

    auto tokenIter = reservedWords.find( id );
    if( tokenIter != reservedWords.end() ) {
        token = tokenIter->second;
    } else {
        token = Tokens::IDENTIFIER;
    }
}

bool Tokenizer::nextChar() {
    if( file[position]=='\n' ) {
        col=0;
        line++;
    }
    position++;
    col++;

    return position<file.length();
}

std::ostream &operator<<(std::ostream &out, Tokenizer::Tokens token) {
#define CASE(name) case Tokenizer::Tokens::name: out<<#name; break
    switch(token) {
        CASE(WS);
        CASE(SEMICOLON);
        CASE(BRACKET_ROUND_OPEN);
        CASE(BRACKET_ROUND_CLOSE);
        CASE(BRACKET_SQUARE_OPEN);
        CASE(BRACKET_SQUARE_CLOSE);
        CASE(BRACKET_CURLY_OPEN);
        CASE(BRACKET_CURLY_CLOSE);
        CASE(LITERAL_INT_2);
        CASE(LITERAL_INT_8);
        CASE(LITERAL_INT_10);
        CASE(LITERAL_INT_16);
        CASE(LITERAL_FP);
        CASE(LITERAL_STRING);
        CASE(IDENTIFIER);
        CASE(RESERVED_DEF);
    }

    return out;
#undef CASE
}
