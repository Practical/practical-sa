#include "parser.h"

#include "scope_tracing.h"

static PracticalSemanticAnalyzer::StaticType toStaticType(const NonTerminals::Type &type) {
    PracticalSemanticAnalyzer::StaticType ret;

    ret.id = type.identId;

    return ret;
}

namespace NonTerminals {

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

size_t Type::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    type = expectToken(Tokenizer::Tokens::IDENTIFIER, source, tokensConsumed, "Expected type",
            "EOF while parsing type" );

    return tokensConsumed;
}

void Type::symbolsPass2(const LookupContext *parent)
{
    auto symbol = parent->getSymbol(type.text);
    if( symbol==nullptr || !symbol->isType() )
        throw pass2_error("Type name expected", type.line, type.col);

    // TODO implement
}

size_t Expression::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    try {
        CompoundExpression compound(ctx);
        tokensConsumed += compound.parse(source, ctx);
        value = safenew<CompoundExpression>(std::move(compound));

        return tokensConsumed;
    } catch( parser_error &err ) {
    }

    assert(tokensConsumed==0);
    nextToken(source, tokensConsumed, "EOF while parsing function arguments");
    const Tokenizer::Token *currentToken = &source[tokensConsumed++];

    switch( currentToken->token ) {
    case Tokenizer::Tokens::IDENTIFIER:
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

    value = source[tokensConsumed];

    return tokensConsumed;
}

size_t Statement::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    tokensConsumed += expression.parse(source, ctx);

    expectToken( Tokenizer::Tokens::SEMICOLON, source, tokensConsumed, "Statement does not end with a semicolon",
            "Unexpected EOF" );

    return tokensConsumed;
}

size_t StatementList::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    try {
        // One of the disadvantages of using exceptions for signalling expected errors is "infinite" loops
        while( true ) {
            Statement statement;
            tokensConsumed += statement.parse(source.subslice(tokensConsumed), ctx);

            statements.emplace_back( std::move(statement) );
        }
    } catch( parser_error &err ) {
    }

    return tokensConsumed;
}

size_t CompoundExpression::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    expectToken( Tokenizer::Tokens::BRACKET_CURLY_OPEN, source, tokensConsumed, "Expected {",
            "EOF while parsing compound statement" );

    tokensConsumed += statementList.parse(source.subslice(tokensConsumed), &context);

    try {
        tokensConsumed += expression.parse(source.subslice(tokensConsumed), &context);
    } catch( parser_error &err ) {
    }

    expectToken( Tokenizer::Tokens::BRACKET_CURLY_CLOSE, source, tokensConsumed, "Expected }",
            "Unmatched {" );

    return tokensConsumed;
}

size_t FuncDeclRet::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    try {
        expectToken(
                Tokenizer::Tokens::OP_ARROW, source, tokensConsumed, "Expected ->",
                "EOF while parsing function return type" );
        // TODO If we found an arrow, probably best to fail if the rest doesn't match

        tokensConsumed += type.parse(source.subslice(tokensConsumed), ctx);
    } catch( parser_error &err ) {
        // Match ϵ
        return 0;
    }

    return tokensConsumed;
}

void FuncDeclRet::symbolsPass2(const LookupContext *ctx) {
    type.symbolsPass2(ctx);
}

size_t FuncDeclArg::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    tokensConsumed += type.parse(source, ctx);
    name = expectToken(
            Tokenizer::Tokens::IDENTIFIER, source, tokensConsumed, "Expected argument name",
            "EOF while parsing function arguments" );

    return tokensConsumed;
}

size_t FuncDeclArgsNonEmpty::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    bool done = false;
    do {
        nextToken(source, tokensConsumed, "EOF while parsing function arguments");

        FuncDeclArg arg;
        tokensConsumed += arg.parse(source, ctx);
        arguments.emplace_back(arg);

        skipWS( source, tokensConsumed );

        if( tokensConsumed < source.size() && source[tokensConsumed].token == Tokenizer::Tokens::COMMA ) {
            tokensConsumed++;
        } else {
            done = true;
        }
    } while( !done );

    return tokensConsumed;
}

size_t FuncDeclArgs::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    try {
        FuncDeclArgsNonEmpty args;
        tokensConsumed += args.parse(source, ctx);
        arguments = std::move(args.arguments);
    } catch( parser_error &err ) {
        // That didn't match - use the empty match rule
    }

    return tokensConsumed;
}

size_t FuncDeclBody::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    nextToken(source, tokensConsumed, "EOF while parsing function declaration");

    name = expectToken(
            Tokenizer::Tokens::IDENTIFIER, source, tokensConsumed, "Expected function name",
            "EOF while parsing function declaration" );

    expectToken( Tokenizer::Tokens::BRACKET_ROUND_OPEN, source, tokensConsumed, "Expected '('",
            "EOF while parsing function declaration" );

    tokensConsumed += arguments.parse( source.subslice(tokensConsumed), ctx );

    expectToken( Tokenizer::Tokens::BRACKET_ROUND_CLOSE, source, tokensConsumed, "Expected ')'",
            "EOF while parsing function declaration" );

    tokensConsumed += returnType.parse( source.subslice(tokensConsumed), ctx );

    return tokensConsumed;
}

void FuncDeclBody::symbolsPass2(const LookupContext *ctx) {
    returnType.symbolsPass2(ctx);
}

size_t FuncDef::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    nextToken(source, tokensConsumed, "EOF while looking for function definition");

    const Tokenizer::Token *currentToken = &source[tokensConsumed];
    if( currentToken->token!=Tokenizer::Tokens::RESERVED_DEF ) {
        throw parser_error("Function definition should start with \"def\"", currentToken->line, currentToken->col);
    }

    tokensConsumed++;

    nextToken(source, tokensConsumed, "EOF while looking for function definition");

    tokensConsumed += decl.parse( source.subslice(tokensConsumed), ctx );

    nextToken(source, tokensConsumed, "EOF while looking for function definition");

    tokensConsumed += body.parse( source.subslice(tokensConsumed), ctx );

    return tokensConsumed;
}

void FuncDef::symbolsPass2(const LookupContext *ctx) {
    decl.symbolsPass2(ctx);
}

void FuncDef::codeGen(PracticalSemanticAnalyzer::ModuleGen *moduleGen) {
    std::shared_ptr<FunctionGen> functionGen = moduleGen->handleFunction(id);

    if( functionGen==nullptr )
        return;

    functionGen->functionEnter(
            id,
            decl.name.text,
            toStaticType(decl.returnType.type),
            Slice<VariableDeclaration>(),
            toSlice("No file"), decl.name.line, decl.name.col);

    // TODO implement

    functionGen->functionLeave(id);
}

void Module::parse(String source) {
    tokens = Tokenizer::tokenize(source);
    parse(tokens, nullptr);
}

size_t Module::parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) {
    size_t tokensConsumed = 0;

    while( tokensConsumed<source.size() ) {
        if( skipWS( source, tokensConsumed ) ) {
            continue;
        }

        FuncDef func(ctx);

        tokensConsumed += func.parse( source.subslice(tokensConsumed), ctx );
        functionDefinitions.emplace_back( std::move(func) );
    }

    return tokensConsumed;
}

void Module::codeGen(PracticalSemanticAnalyzer::ModuleGen *codeGen) {
    codeGen->moduleEnter(id, toSlice("OnlyModule"), toSlice("No file"), 1, 1);

    for(auto &function: functionDefinitions) {
        function.codeGen(codeGen);
    }

    codeGen->moduleLeave(id);
}

} // namespace NonTerminals
