#include <tokenizer.h>

#include "asserts.h"
#include "practical-errors.h"

#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

using PracticalSemanticAnalyzer::tokenizer_error;

namespace Tokenizer {

static const std::unordered_set<char> operatorChars = {
    '~', '!', '#', '/', '$', '%', '^', '&', '*', '-', '=', '+', '<', '>', '.', '|', ':' };
static const std::unordered_map<std::string, Tokens> operators {
    { "=", Tokens::OP_ASSIGN },
    { "!", Tokens::OP_LOGIC_NOT },
    { "%", Tokens::OP_MODULOUS },
    { "&", Tokens::OP_BIT_AND },
    { "&&", Tokens::OP_LOGIC_AND },
    { "*", Tokens::OP_ASTERISK },
    { "-", Tokens::OP_MINUS },
    { "--", Tokens::OP_MINUS_MINUS },
    { "-=", Tokens::OP_ASSIGN_MINUS },
    { "->", Tokens::OP_ARROW },
    { ":", Tokens::OP_COLON },
};
static const std::unordered_map<std::string, Tokens> reservedWords { { "def", Tokens::RESERVED_DEF } };


bool Tokenizer::next() {
    if( file.size()==position ) {
        // We've reached our EOF
        tokenLine = line;
        tokenCol = col;
        return false;
    }

    token = Tokens::ERR;

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
    } else if(currentChar=='/') {
        consumePossibleComment();
    } else if(operatorChars.find(currentChar) != operatorChars.end()) {
        consumeOp();
    } else if(currentChar=='"') {
        consumeStringLiteral();
    } else if(isDigit(currentChar)) {
        consumeNumericLiteral();
    } else if(isIdentifierAlpha(currentChar)) {
        consumeIdentifier();
    } else {
        tokenText = file.subslice(tokenStart, tokenStart+1);
        throw tokenizer_error("Invalid character encountered", startLine, startCol);
    }
    tokenText = file.subslice(tokenStart, position);
    tokenLine = startLine;
    tokenCol = startCol;
    ASSERT( token!=Tokens::ERR );

    return true;
}

std::vector<Token> Tokenizer::tokenize(String source) {
    std::vector<Token> tokens;

    Tokenizer tokenizer(source);

    while( tokenizer.next() ) {
        tokens.push_back(tokenizer.current());
    }

    return tokens;
}

void Tokenizer::consumeWS() {
    token = Tokens::WS;
    while( nextChar() && isWS(file[position]) )
        ;
}

void Tokenizer::consumeOp() {
    auto startPosition = position;
    auto startLine = line, startCol = col;
    while( nextChar() && operatorChars.find(file[position])!=operatorChars.end() )
        ;

    auto op = file.subslice(startPosition, position);
    auto tokenIt = operators.find( sliceToString(op) );
    if( tokenIt != operators.end() ) {
        token = tokenIt->second;
    } else {
        // Man am I going to regret this error message
        throw tokenizer_error("Practical does not support inventing weird operators", startLine, startCol);
    }
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

void Tokenizer::consumePossibleComment() {
    auto savedPosition = savePosition();

    if( !nextChar() ) {
        // EOF - Not a comment
        restorePosition(savedPosition);
        return consumeOp();
    }

    switch( file[position] ) {
    case '/':
        consumeLineComment();
        break;
    case '*':
        consumeNestableComment(savedPosition);
        break;
    default:
        restorePosition(savedPosition);
        consumeOp();
    }
}

void Tokenizer::consumeLineComment() {
    token = Tokens::COMMENT;
    while( nextChar() && file[position]!='\n' )
        ;
}

void Tokenizer::consumeNestableComment(SavedPoint startPoint) {
    token = Tokens::COMMENT;

    while( nextChar() ) {
        SavedPoint recursiveStartPoint = savePosition();

        // Nested comment
        if( file[position]=='/' && nextChar() && file[position]=='*' ) {
            consumeNestableComment(recursiveStartPoint);
        } else if( file[position]=='*' && nextChar() && file[position]=='/' ) {
            // Comment end
            nextChar();
            return;
        }
    }

    throw tokenizer_error("Unterminated multi-line comment", startPoint.line, startPoint.col);
}

bool Tokenizer::nextChar() {
    // Allow continued calling after EOF already reached
    if( position>=file.size() )
        return false;

    if( file[position]=='\n' ) {
        col=0;
        line++;
    }
    position++;
    col++;

    return position<file.size();
}

Tokenizer::SavedPoint Tokenizer::savePosition() {
    return SavedPoint{ .line=line, .col=col, .position=position };
}

void Tokenizer::restorePosition(Tokenizer::SavedPoint position) {
    line = position.line;
    col = position.col;
    this->position = position.position;
}

} // namespace Tokenizer

std::ostream &operator<<(std::ostream &out, Tokenizer::Tokens token) {
#define CASE(name) case Tokenizer::Tokens::name: out<<#name; break
    switch(token) {
        CASE(ERR);
        CASE(WS);
        CASE(COMMENT);
        CASE(SEMICOLON);
        CASE(COMMA);
        CASE(BRACKET_ROUND_OPEN);
        CASE(BRACKET_ROUND_CLOSE);
        CASE(BRACKET_SQUARE_OPEN);
        CASE(BRACKET_SQUARE_CLOSE);
        CASE(BRACKET_CURLY_OPEN);
        CASE(BRACKET_CURLY_CLOSE);
        CASE(OP_ASTERISK);
        CASE(OP_ARROW);
        CASE(OP_LOGIC_NOT);
        CASE(OP_LOGIC_AND);
        CASE(OP_LOGIC_OR);
        CASE(OP_MODULOUS);
        CASE(OP_MINUS);
        CASE(OP_MINUS_MINUS);
        CASE(OP_PLUS);
        CASE(OP_PLUS_PLUS);
        CASE(OP_BIT_AND);
        CASE(OP_BIT_OR);
        CASE(OP_BIT_NOT);
        CASE(OP_ASSIGN);
        CASE(OP_ASSIGN_MINUS);
        CASE(OP_ASSIGN_PLUS);
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
