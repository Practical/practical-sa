/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
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
    '~', '!', '#', '/', '$', '%', '^', '&', '*', '-', '=', '+', '<', '>', '.', '|', ':', '@' };
static const std::unordered_map<String, Tokens> operators {
    // Precedence 1
    { "::", Tokens::OP_DOUBLE_COLON },
    // Precedence 2
    { "++", Tokens::OP_PLUS_PLUS },
    { "--", Tokens::OP_MINUS_MINUS },
    { "(", Tokens::BRACKET_ROUND_OPEN },
    { ")", Tokens::BRACKET_ROUND_CLOSE },
    { "[", Tokens::BRACKET_SQUARE_OPEN },
    { "]", Tokens::BRACKET_SQUARE_CLOSE },
    { ".", Tokens::OP_DOT },
    { "->", Tokens::OP_ARROW },
    { "@", Tokens::OP_PTR },
    // Precedence 3
    { "+", Tokens::OP_PLUS },
    { "-", Tokens::OP_MINUS },
    { "~", Tokens::OP_BIT_NOT },
    { "!", Tokens::OP_LOGIC_NOT },
    { "*", Tokens::OP_MULTIPLY },
    { "&", Tokens::OP_AMPERSAND },
    // Precedence 5
    { "/", Tokens::OP_DIVIDE },
    { "%", Tokens::OP_MODULOUS },
    // Precedence 6
    { "|", Tokens::OP_BIT_OR },
    { "^", Tokens::OP_BIT_XOR },
    // Precedence 8
    { "<<", Tokens::OP_SHIFT_LEFT },
    { ">>", Tokens::OP_SHIFT_RIGHT },
    // Precedence 9
    { "<", Tokens::OP_LESS_THAN },
    { "<=", Tokens::OP_LESS_THAN_EQ },
    { ">", Tokens::OP_GREATER_THAN },
    { ">=", Tokens::OP_GREATER_THAN_EQ },
    // Precedence 10
    { "==", Tokens::OP_EQUALS },
    { "!=", Tokens::OP_NOT_EQUALS },
    // Precedence 11
    { "&&", Tokens::OP_LOGIC_AND },
    // Precedence 12
    { "||", Tokens::OP_LOGIC_OR },
    // Precedence 13
    { "=", Tokens::OP_ASSIGN },
    { "+=", Tokens::OP_ASSIGN_PLUS },
    { "-=", Tokens::OP_ASSIGN_MINUS },
    { "*=", Tokens::OP_ASSIGN_MULTIPLY },
    { "/=", Tokens::OP_ASSIGN_DIVIDE },
    { "%=", Tokens::OP_ASSIGN_MODULOUS },
    { "<<=", Tokens::OP_ASSIGN_LEFT_SHIFT },
    { ">>=", Tokens::OP_ASSIGN_RIGHT_SHIFT },
    { "&=", Tokens::OP_ASSIGN_BIT_AND },
    { "^=", Tokens::OP_ASSIGN_BIT_XOR },
    { "|=", Tokens::OP_ASSIGN_BIT_OR },

    // Miscellany
    { "+++", Tokens::OP_RUNON_ERROR },
    { "---", Tokens::OP_RUNON_ERROR },
    { "&=", Tokens::OP_ASSIGN_BIT_AND },
    { ":", Tokens::OP_COLON },
    { "//", Tokens::COMMENT_LINE_END },
    { "/*", Tokens::COMMENT_MULTILINE },
};
static const std::unordered_map<std::string, Tokens> reservedWords {
    { "def", Tokens::RESERVED_DEF },
    { "expect", Tokens::RESERVED_EXPECT },
    { "true", Tokens::RESERVED_TRUE },
    { "false", Tokens::RESERVED_FALSE },
    { "if", Tokens::RESERVED_IF },
    { "else", Tokens::RESERVED_ELSE },
};


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
    } else if(currentChar==',') {
        nextChar();
        token = Tokens::COMMA;
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

    auto lastIdentified = savePosition();
    bool found = false;
    bool foundLastRound = false;

    do {
        if( foundLastRound ) {
            lastIdentified = savePosition();
            foundLastRound = false;
        }

        auto op = file.subslice(startPosition, position+1);

        auto opIter = operators.find(op);
        if( opIter != operators.end() ) {
            token = opIter->second;
            found = true;
            foundLastRound = true;
        }
    } while( nextChar() && operatorChars.find(file[position])!=operatorChars.end() );

    if( foundLastRound ) {
        lastIdentified = savePosition();
        foundLastRound = false;
    }

    if( !found ) {
        // Man am I going to regret this error message
        throw tokenizer_error("Practical does not support inventing weird operators", startLine, startCol);
    }

    restorePosition( lastIdentified );

    switch( token ) {
    case Tokens::ERR:
        ABORT() << "consumeOp found operator \"" << file.subslice(startPosition, position) << "\" at " << startLine << ":" <<
                startCol << " but token returned was ERR";
        break;
    case Tokens::OP_RUNON_ERROR:
        throw tokenizer_error(
                "The compiler refuses to guess which combination of operators you meant. Disambiguate the code with spaces",
                startLine, startCol);
        break;
    case Tokens::COMMENT_LINE_END:
        consumeLineComment();
        break;
    case Tokens::COMMENT_MULTILINE:
        consumeNestableComment( SavedPoint{ .line=startLine, .col=startCol, .position=startPosition } );
        break;
    default:
        // Successful matching, nothing else to do
        break;
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

    const char *start = &file[startPos];
    // The strange math is to avoid violating bounds checks in case this is the last thing in the file
    const char *end = file.get() + position;
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

void Tokenizer::consumeLineComment() {
    while( nextChar() && file[position]!='\n' )
        ;
}

void Tokenizer::consumeNestableComment(SavedPoint startPoint) {
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
#define CASE(name) case Tokenizer::Tokens::name: out<<#name; return out
    switch(token) {
        CASE(ERR);
        CASE(WS);
        CASE(COMMENT_MULTILINE);
        CASE(COMMENT_LINE_END);
        CASE(SEMICOLON);
        CASE(COMMA);
        CASE(BRACKET_ROUND_OPEN);
        CASE(BRACKET_ROUND_CLOSE);
        CASE(BRACKET_SQUARE_OPEN);
        CASE(BRACKET_SQUARE_CLOSE);
        CASE(BRACKET_CURLY_OPEN);
        CASE(BRACKET_CURLY_CLOSE);
        CASE(OP_AMPERSAND);
        CASE(OP_ARROW);
        CASE(OP_ASSIGN);
        CASE(OP_ASSIGN_BIT_AND);
        CASE(OP_ASSIGN_BIT_OR);
        CASE(OP_ASSIGN_BIT_XOR);
        CASE(OP_ASSIGN_DIVIDE);
        CASE(OP_ASSIGN_LEFT_SHIFT);
        CASE(OP_ASSIGN_MINUS);
        CASE(OP_ASSIGN_MODULOUS);
        CASE(OP_ASSIGN_MULTIPLY);
        CASE(OP_ASSIGN_PLUS);
        CASE(OP_ASSIGN_RIGHT_SHIFT);
        CASE(OP_ASSIGN_RIGHT_SHIFT_LOGICAL);
        CASE(OP_MULTIPLY);
        CASE(OP_BIT_AND);
        CASE(OP_BIT_NOT);
        CASE(OP_BIT_OR);
        CASE(OP_BIT_XOR);
        CASE(OP_PTR);
        CASE(OP_COLON);
        CASE(OP_DIVIDE);
        CASE(OP_DOT);
        CASE(OP_DOUBLE_COLON);
        CASE(OP_EQUALS);
        CASE(OP_GREATER_THAN);
        CASE(OP_GREATER_THAN_EQ);
        CASE(OP_LESS_THAN);
        CASE(OP_LESS_THAN_EQ);
        CASE(OP_LOGIC_AND);
        CASE(OP_LOGIC_NOT);
        CASE(OP_LOGIC_OR);
        CASE(OP_MINUS);
        CASE(OP_MINUS_MINUS);
        CASE(OP_MODULOUS);
        CASE(OP_NOT_EQUALS);
        CASE(OP_PLUS);
        CASE(OP_PLUS_PLUS);
        CASE(OP_SHIFT_LEFT);
        CASE(OP_SHIFT_RIGHT);
        CASE(OP_RUNON_ERROR);
        CASE(LITERAL_INT_2);
        CASE(LITERAL_INT_8);
        CASE(LITERAL_INT_10);
        CASE(LITERAL_INT_16);
        CASE(LITERAL_FP);
        CASE(LITERAL_STRING);
        CASE(IDENTIFIER);
        CASE(RESERVED_DEF);
        CASE(RESERVED_EXPECT);
        CASE(RESERVED_TRUE);
        CASE(RESERVED_FALSE);
        CASE(RESERVED_IF);
        CASE(RESERVED_ELSE);
    }

    out<<"Tokens("<<static_cast<int>(token)<<")";

    return out;
#undef CASE
}

std::ostream &operator<<(std::ostream &out, const Tokenizer::Token &token) {
    out<<token.token<<" ("<<token.text<<") at "<<token.line<<":"<<token.col;
    return out;
}
