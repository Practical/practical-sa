#include "parser.h"

#include "practical-errors.h"
#include "scope_tracing.h"

namespace NonTerminals {

#if VERBOSE_PARSING
static size_t RECURSION_DEPTH;

#define RULE_ENTER(source) \
    size_t RECURSION_CURRENT_DEPTH = RECURSION_DEPTH++; \
    for( size_t I=0; I<RECURSION_CURRENT_DEPTH; ++I ) std::cout<<"  "; \
    std::cout<<"Processing " << __PRETTY_FUNCTION__ << " of " << source[0].token << " at " << source[0].line << ":" << source[0].col << "\n"

#define RULE_LEAVE(tokensConsumed) \
    RECURSION_DEPTH = RECURSION_CURRENT_DEPTH; \
    for( size_t I=0; I<RECURSION_CURRENT_DEPTH; ++I ) std::cout<<"  "; \
    std::cout<<"Leaving " << __PRETTY_FUNCTION__ << " consumed " << tokensConsumed << "\n"; \
    return tokensConsumed

#define EXCEPTION_CAUGHT(ex) \
    RECURSION_DEPTH = RECURSION_CURRENT_DEPTH + 1; \
    for( size_t I=0; I<RECURSION_CURRENT_DEPTH; ++I ) std::cout<<"  "; \
    std::cout<< __PRETTY_FUNCTION__ << " caught " << ex.what() << "\n"

#else

#define RULE_ENTER(source)
#define RULE_LEAVE(tokensConsumed) return tokensConsumed
#define EXCEPTION_CAUGHT(ex)

#endif

static bool skipWS(Slice<const Tokenizer::Token> source, size_t &index) {
    bool moved = false;

    while( index<source.size() &&
            (source[index].token == Tokenizer::Tokens::WS || source[index].token == Tokenizer::Tokens::COMMENT) )
    {
        index++;
        moved = true;
    }

    return moved;
}

// Sets the index to the next token, verifying EOF and skipping WS if necessary
static void nextToken(Slice<const Tokenizer::Token> source, size_t &index, const char *msg, bool skipWS = true) {
    if( skipWS )
        NonTerminals::skipWS(source, index);

    if( index==source.size() )
        throw parser_error(msg, 0, 0);
}

static const Tokenizer::Token &expectToken(
        Tokenizer::Tokens expected, Slice<const Tokenizer::Token> source, size_t &index, const char *mismatchMsg,
        const char *eofMsg)
{
    nextToken( source, index, eofMsg );

    const Tokenizer::Token *currentToken = &source[index];
    if( currentToken->token!=expected ) {
        throw parser_error(mismatchMsg, currentToken->line, currentToken->col);
    }

    index++;

    return *currentToken;
}

bool wishForToken(Tokenizer::Tokens expected, Slice<const Tokenizer::Token> source, size_t &index) {
    skipWS( source, index );

    if( index>=source.size() ) {
        return false;
    }

    if( source[index].token == expected ) {
        index++;

        return true;
    }

    return false;
}

size_t Identifier::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    size_t tokensConsumed = 0;

    identifier = &expectToken(Tokenizer::Tokens::IDENTIFIER, source, tokensConsumed, "Expected an identifier",
            "EOF while parsing an identifier" );

    RULE_LEAVE(tokensConsumed);
}

size_t Type::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    return type.parse(source);
}

size_t Literal::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    nextToken(source, tokensConsumed, "EOF while parsing literal");
    const Tokenizer::Token *currentToken = &source[tokensConsumed++];

    switch( currentToken->token ) {
    case Tokenizer::Tokens::LITERAL_INT_2:
    case Tokenizer::Tokens::LITERAL_INT_8:
    case Tokenizer::Tokens::LITERAL_INT_10:
    case Tokenizer::Tokens::LITERAL_INT_16:
    case Tokenizer::Tokens::LITERAL_FP:
    case Tokenizer::Tokens::LITERAL_STRING:
        break;
    default:
        throw parser_error("Invalid expression", currentToken->line, currentToken->col);
    }

    token = *currentToken;

    RULE_LEAVE(tokensConsumed);
}

size_t Expression::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    try {
        ::NonTerminals::CompoundExpression compound;
        tokensConsumed += compound.parse(source);
        value = safenew<::NonTerminals::CompoundExpression>(std::move(compound));

        RULE_LEAVE(tokensConsumed);
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
    }

    ASSERT(tokensConsumed==0);

    try {
        // Maybe a Literal
        value.emplace<Literal>();
        tokensConsumed += std::get<Literal>(value).parse(source);

        RULE_LEAVE(tokensConsumed);
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
    }

    ASSERT(tokensConsumed==0);

    tokensConsumed += value.emplace<Identifier>().parse( source.subslice(tokensConsumed) );

    RULE_LEAVE(tokensConsumed);
}

size_t VariableDeclBody::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    tokensConsumed += name.parse(source);
    expectToken( Tokenizer::Tokens::OP_COLON, source, tokensConsumed, "Expected \":\" after variable name", "Unexpected EOF" );
    tokensConsumed += type.parse( source.subslice(tokensConsumed) );

    RULE_LEAVE(tokensConsumed);
}

size_t VariableDefinition::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    expectToken( Tokenizer::Tokens::RESERVED_DEF, source, tokensConsumed, "Variable definition does not start with def keyword",
            "Unexpected EOF" );

    tokensConsumed += body.parse(source.subslice(tokensConsumed));

    try {
        size_t provisionalConsumed = tokensConsumed;

        expectToken( Tokenizer::Tokens::OP_ASSIGN, source, provisionalConsumed, "", "" );

        // XXX parse outside the catch
        Expression initValue;
        provisionalConsumed += initValue.parse( source.subslice(provisionalConsumed) );

        this->initValue = safenew<Expression>( std::move(initValue) );
        tokensConsumed = provisionalConsumed;
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
    }

    RULE_LEAVE(tokensConsumed);
}

size_t Statement::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    try {
        Expression expression;
        size_t tokensConsumed = expression.parse(source);

        expectToken( Tokenizer::Tokens::SEMICOLON, source, tokensConsumed, "Statement does not end with a semicolon",
                "Unexpected EOF" );

        content = std::move(expression);

        RULE_LEAVE(tokensConsumed);
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
    }

    size_t tokensConsumed = 0;

    VariableDefinition def;

    tokensConsumed += def.parse(source);
    expectToken( Tokenizer::Tokens::SEMICOLON, source, tokensConsumed, "Statement does not end with a semicolon",
            "Unexpected EOF" );

    content = std::move(def);
    RULE_LEAVE(tokensConsumed);
}

size_t StatementList::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    try {
        // One of the disadvantages of using exceptions for signalling expected errors is "infinite" loops
        while( true ) {
            Statement statement;
            tokensConsumed += statement.parse(source.subslice(tokensConsumed));

            statements.emplace_back( std::move(statement) );
        }
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
    }

    RULE_LEAVE(tokensConsumed);
}

size_t CompoundExpression::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    expectToken( Tokenizer::Tokens::BRACKET_CURLY_OPEN, source, tokensConsumed, "Expected {",
            "EOF while parsing compound statement" );

    tokensConsumed += statementList.parse(source.subslice(tokensConsumed));

    try {
        tokensConsumed += expression.parse(source.subslice(tokensConsumed));
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
    }

    expectToken( Tokenizer::Tokens::BRACKET_CURLY_CLOSE, source, tokensConsumed, "Expected }",
            "Unmatched {" );

    RULE_LEAVE(tokensConsumed);
}

size_t FuncDeclRet::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    try {
        expectToken(
                Tokenizer::Tokens::OP_ARROW, source, tokensConsumed, "Expected ->",
                "EOF while parsing function return type" );
        // TODO If we found an arrow, probably best to fail if the rest doesn't match

        tokensConsumed += type.parse(source.subslice(tokensConsumed));
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
        // Match ϵ
        return 0;
    }

    RULE_LEAVE(tokensConsumed);
}

size_t FuncDeclArg::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    tokensConsumed += name.parse( source.subslice(tokensConsumed) );
    expectToken(
            Tokenizer::Tokens::OP_COLON, source, tokensConsumed, "Expected colon in argument declaration",
            "EOF while parsing function declaration" );
    tokensConsumed += type.parse( source.subslice(tokensConsumed) );

    RULE_LEAVE(tokensConsumed);
}

size_t FuncDeclArgsNonEmpty::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    bool done = false;
    do {
        nextToken(source, tokensConsumed, "EOF while parsing function arguments");

        FuncDeclArg arg;
        tokensConsumed += arg.parse(source);
        arguments.emplace_back(arg);

        skipWS( source, tokensConsumed );

        if( tokensConsumed < source.size() && source[tokensConsumed].token == Tokenizer::Tokens::COMMA ) {
            tokensConsumed++;
        } else {
            done = true;
        }
    } while( !done );

    RULE_LEAVE(tokensConsumed);
}

size_t FuncDeclArgs::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    try {
        FuncDeclArgsNonEmpty args;
        tokensConsumed += args.parse(source);
        arguments = std::move(args.arguments);
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
        // That didn't match - use the empty match rule
    }

    RULE_LEAVE(tokensConsumed);
}

size_t FuncDeclBody::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    nextToken(source, tokensConsumed, "EOF while parsing function declaration");

    tokensConsumed += name.parse( source  );

    expectToken( Tokenizer::Tokens::BRACKET_ROUND_OPEN, source, tokensConsumed, "Expected '('",
            "EOF while parsing function declaration" );

    tokensConsumed += arguments.parse( source.subslice(tokensConsumed) );

    expectToken( Tokenizer::Tokens::BRACKET_ROUND_CLOSE, source, tokensConsumed, "Expected ')'",
            "EOF while parsing function declaration" );

    tokensConsumed += returnType.parse( source.subslice(tokensConsumed) );

    RULE_LEAVE(tokensConsumed);
}

size_t FuncDef::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    nextToken(source, tokensConsumed, "EOF while looking for function definition");

    const Tokenizer::Token *currentToken = &source[tokensConsumed];
    if( currentToken->token!=Tokenizer::Tokens::RESERVED_DEF ) {
        throw parser_error("Function definition should start with \"def\"", currentToken->line, currentToken->col);
    }

    tokensConsumed++;

    nextToken(source, tokensConsumed, "EOF while looking for function definition");

    tokensConsumed += decl.parse( source.subslice(tokensConsumed) );

    nextToken(source, tokensConsumed, "EOF while looking for function definition");

    tokensConsumed += body.parse( source.subslice(tokensConsumed) );

    RULE_LEAVE(tokensConsumed);
}

void Module::parse(String source) {
    tokens = Tokenizer::Tokenizer::tokenize(source);
    parse(tokens);
}

size_t Module::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    size_t tokensConsumed = 0;

    while( tokensConsumed<source.size() ) {
        if( skipWS( source, tokensConsumed ) ) {
            continue;
        }

        FuncDef func;

        tokensConsumed += func.parse( source.subslice(tokensConsumed) );
        functionDefinitions.emplace_back( std::move(func) );
    }

    RULE_LEAVE(tokensConsumed);
}

} // namespace NonTerminals
