#include "parser.h"

#include "practical-errors.h"
#include "scope_tracing.h"

namespace NonTerminals {

#if VERBOSE_PARSING
static size_t RECURSION_DEPTH;

#define RULE_ENTER(source) \
    size_t RECURSION_CURRENT_DEPTH = RECURSION_DEPTH++; \
    for( size_t I=0; I<RECURSION_CURRENT_DEPTH; ++I ) std::cout<<"  "; \
    if( source.size()>0 ) \
        std::cout<<"Processing " << __PRETTY_FUNCTION__ << " of " << source[0].token << " at " << source[0].line << ":" << source[0].col << "\n" ;\
    else\
        std::cout<<"Processing " << __PRETTY_FUNCTION__ << " at EOF\n" ;\
    size_t tokensConsumed = 0

#define RULE_LEAVE() \
    RECURSION_DEPTH = RECURSION_CURRENT_DEPTH; \
    for( size_t I=0; I<RECURSION_CURRENT_DEPTH; ++I ) std::cout<<"  "; \
    std::cout<<"Leaving " << __PRETTY_FUNCTION__ << " consumed " << tokensConsumed << "\n"; \
    return tokensConsumed

#define EXCEPTION_CAUGHT(ex) \
    RECURSION_DEPTH = RECURSION_CURRENT_DEPTH + 1; \
    for( size_t I=0; I<RECURSION_CURRENT_DEPTH; ++I ) std::cout<<"  "; \
    std::cout<< __PRETTY_FUNCTION__ << " caught " << ex.what() << "\n"

#else

#define RULE_ENTER(source) size_t tokensConsumed = 0
#define RULE_LEAVE() return tokensConsumed
#define EXCEPTION_CAUGHT(ex)

#endif

static bool skipWS(Slice<const Tokenizer::Token> source, size_t &index) {
    bool moved = false;

    while( index<source.size() &&
            (source[index].token == Tokenizer::Tokens::WS || source[index].token == Tokenizer::Tokens::COMMENT_LINE_END ||
             source[index].token == Tokenizer::Tokens::COMMENT_MULTILINE) )
    {
        index++;
        moved = true;
    }

    return moved;
}

// Consumes the next token, optionally reporting EOF
static const Tokenizer::Token *nextToken(Slice<const Tokenizer::Token> source, size_t &index, const char *msg = nullptr) {
    skipWS(source, index);

    if( index==source.size() ) {
        if( msg==nullptr )
            return nullptr;
        else
            throw parser_error(msg, 0, 0);
    }

    return &source[index++];
}

static const Tokenizer::Token &expectToken(
        Tokenizer::Tokens expected, Slice<const Tokenizer::Token> source, size_t &index, const char *mismatchMsg,
        const char *eofMsg)
{
    const Tokenizer::Token *currentToken = nextToken( source, index, eofMsg );
    ASSERT( currentToken!=nullptr ) << "Unexpected EOF condition when searching for token";

    if( currentToken->token!=expected ) {
        index--;
        throw parser_error(mismatchMsg, currentToken->line, currentToken->col);
    }

    return *currentToken;
}

bool wishForToken(Tokenizer::Tokens expected, Slice<const Tokenizer::Token> source, size_t &index) {
    size_t indexCopy = index;
    skipWS( source, indexCopy );

    if( indexCopy>=source.size() ) {
        return false;
    }

    if( source[indexCopy].token == expected ) {
        index = indexCopy + 1;

        return true;
    }

    return false;
}

size_t Identifier::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    identifier = &expectToken(Tokenizer::Tokens::IDENTIFIER, source, tokensConsumed, "Expected an identifier",
            "EOF while parsing an identifier" );

    RULE_LEAVE();
}

size_t Type::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    tokensConsumed = type.parse(source);
    RULE_LEAVE();
}

size_t Literal::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    const Tokenizer::Token *currentToken = nextToken(source, tokensConsumed, "EOF while parsing literal");

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

    RULE_LEAVE();
}

size_t FunctionArguments::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    bool firstArgument = true;
    while( !wishForToken(Tokenizer::Tokens::BRACKET_ROUND_CLOSE, source, tokensConsumed) ) {
        if( firstArgument ) {
            firstArgument = false;
        } else {
            expectToken(
                    Tokenizer::Tokens::COMMA, source, tokensConsumed, "Function argument list needs to be delimited by commas",
                    "EOF while scanning arguments list" );
        }

        Expression *argument = &arguments.emplace_back();
        tokensConsumed += argument->parse( source.subslice(tokensConsumed) );
    }

    RULE_LEAVE();
}

size_t Expression::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    tokensConsumed = actualParse(source, Operators::operators.size());

    RULE_LEAVE();
}

size_t Expression::actualParse(Slice<const Tokenizer::Token> source, size_t level) {
    using namespace Operators;

    RULE_ENTER(source);

    if( level==0 ) {
        tokensConsumed += basicParse(source);

        RULE_LEAVE();
    }

    // Operators
    const OperatorPriority &priority = operators.at( level-1 );

    switch( priority.kind ) {
    case OperatorPriority::OpKind::Prefix:
        tokensConsumed += parsePrefixOp( source, level, priority.operators );
        break;
    case OperatorPriority::OpKind::Infix:
        tokensConsumed += parseInfixOp( source, level, priority.operators );
        break;
    case OperatorPriority::OpKind::InfixRight2Left:
        tokensConsumed += parseInfixR2LOp( source, level, priority.operators );
        break;
    case OperatorPriority::OpKind::Postfix:
        tokensConsumed += parsePostfixOp( source, level, priority.operators );
        break;
    }

    RULE_LEAVE();
}

size_t Expression::basicParse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    // Parenthesis around expression?
    if( wishForToken( Tokenizer::Tokens::BRACKET_ROUND_OPEN, source, tokensConsumed ) ) {
        tokensConsumed += parse( source.subslice(tokensConsumed) );
        expectToken( Tokenizer::Tokens::BRACKET_ROUND_CLOSE, source, tokensConsumed, "Unmatched (", "EOF searching for )" );

        RULE_LEAVE();
    }

    try {
        // Maybe an identifier
        tokensConsumed += value.emplace<Identifier>().parse(source);

        RULE_LEAVE();
    } catch( parser_error &err ) {
    }

    // Or maybe a Literal
    tokensConsumed = value.emplace<Literal>().parse(source);

    RULE_LEAVE();
}

size_t Expression::parsePrefixOp(
        Slice<const Tokenizer::Token> source, size_t level, const Operators::OperatorPriority::OperatorsMap &operators)
{
    RULE_ENTER(source);

    UnaryOperator &op = value.emplace< UnaryOperator >();
    op.op = nextToken( source, tokensConsumed, "End of file while looking for operator" );
    auto operatorInfo = operators.find( op.op->token );

    if( operatorInfo!=operators.end() ) {
        ASSERT( operatorInfo->second==Operators::OperatorType::Regular );
        // The operator we found is a valid prefix operator for this level
        op.operand = safenew< Expression >();
        tokensConsumed += op.operand->parsePrefixOp( source.subslice(tokensConsumed), level, operators );
        RULE_LEAVE();
    }

    // The use of = instead of += undoes the call to "nextToken" above, as does the use of "source" with no subslicing
    tokensConsumed = actualParse( source, level-1 );

    RULE_LEAVE();
}

size_t Expression::parseInfixOp(
        Slice<const Tokenizer::Token> source, size_t level, const Operators::OperatorPriority::OperatorsMap &operators)
{
    RULE_ENTER(source);

    /*
     * In regular grammer, this rule should say ExpressionN (where N is the level) -> ExpressionN OP ExpressionN-1
     * The direct translation to code of the above is to start by calling op.operand1->actualParse( source, level )
     * Doing this will result in an unterminated recursion (calling ourselves with the precise same arguments).
     *
     * Instead, We're parsing ExpressionN -> ExpressionN-1 OP ExpressionN-1, and if the next token after we're done is an operator
     * of our level, we incorporate that entire previous expression as the right hand side of the higher expression.
     *
     * Rinse, repeat, and we've translated the recursive definition into a loop.
     *
     * One side effect is that the same operator cannot be used as binary operator in different levels. THAT IS A GOOD THING!
     */

    BinaryOperator op;
    op.operand1 = safenew<Expression>();
    tokensConsumed = op.operand1->actualParse( source, level-1 );

    size_t provisionalTokensConsumed = tokensConsumed;
    op.op = nextToken( source, provisionalTokensConsumed );
    if( op.op==nullptr ) {
        *this = std::move(* op.operand1);
        RULE_LEAVE();
    }

    auto opInfo = operators.find(op.op->token);
    if( opInfo==operators.end() ) {
        // This is not the operator you're looking for. Just make do with what we have without it
        *this = std::move(* op.operand1);
        RULE_LEAVE();
    }

    ASSERT( opInfo->second==Operators::OperatorType::Regular );
    tokensConsumed = provisionalTokensConsumed;

    op.operand2 = safenew< Expression >();
    tokensConsumed += op.operand2->actualParse( source.subslice(tokensConsumed), level-1 );

    while( true ) {
        BinaryOperator op2;

        // If the next token is an operator of this level, we need to extend our parse tree to cover it as well
        provisionalTokensConsumed = tokensConsumed;
        op2.op = nextToken( source, provisionalTokensConsumed );
        if( op2.op==nullptr || operators.find( op2.op->token ) == operators.end() ) {
            break;
        }

        // Commit to extending
        tokensConsumed = provisionalTokensConsumed;
        op2.operand1 = safenew< Expression >();
        op2.operand1->value = std::move( op );
        op = std::move( op2 );

        op.operand2 = safenew< Expression >();
        tokensConsumed += op.operand2->actualParse( source.subslice( tokensConsumed ), level-1 );
    }

    value = std::move(op);

    RULE_LEAVE();
}

size_t Expression::parseInfixR2LOp(
        Slice<const Tokenizer::Token> source, size_t level, const Operators::OperatorPriority::OperatorsMap &operators)
{
    RULE_ENTER(source);

    BinaryOperator op;
    op.operand1 = safenew< Expression >();
    tokensConsumed += op.operand1->actualParse( source, level-1 );

    size_t provisionalTokensConsumed = tokensConsumed;
    op.op = nextToken( source, provisionalTokensConsumed );
    if( op.op==nullptr ) {
        *this = std::move(* op.operand1);
        RULE_LEAVE();
    }

    auto opInfo = operators.find(op.op->token);
    if( opInfo==operators.end() ) {
        // This is not the operator you're looking for. Just make do with what we have without it
        *this = std::move(* op.operand1);
        RULE_LEAVE();
    }
    ASSERT( opInfo->second==Operators::OperatorType::Regular );

    tokensConsumed = provisionalTokensConsumed;

    op.operand2 = safenew< Expression >();
    tokensConsumed += op.operand2->actualParse( source.subslice(tokensConsumed), level );

    value = std::move( op );

    RULE_LEAVE();
}

size_t Expression::parsePostfixOp(
        Slice<const Tokenizer::Token> source, size_t level, const Operators::OperatorPriority::OperatorsMap &operators)
{
    RULE_ENTER(source);

    tokensConsumed = actualParse( source, level-1 );

    UnaryOperator op;
    size_t provisionalTokensConsumed = tokensConsumed;
    op.op = nextToken( source, provisionalTokensConsumed );
    if( op.op==nullptr ) {
        RULE_LEAVE();
    }

    auto opInfo = operators.find( op.op->token );
    while( opInfo!=operators.end() ) {
        tokensConsumed = provisionalTokensConsumed;

        switch( opInfo->second ) {
        case Operators::OperatorType::Regular:
            op.operand = safenew< Expression >( std::move( *this ) );
            value = std::move( op );
            ASSERT( ! op.operand );
            break;
        case Operators::OperatorType::Function:
            {
                FunctionCall funcCall;
                funcCall.expression = safenew< Expression >( std::move( *this ) );
                tokensConsumed += funcCall.arguments.parse( source.subslice(tokensConsumed) );
                value = std::move( funcCall );
            }
            break;
        case Operators::OperatorType::SliceSubscript:
            ABORT() << "TODO implement";
        }

        provisionalTokensConsumed = tokensConsumed;
        op.op = nextToken( source, provisionalTokensConsumed );
        if( op.op==nullptr )
            break;
        opInfo = operators.find( op.op->token );
    }

    RULE_LEAVE();
}

size_t VariableDeclBody::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    tokensConsumed += name.parse(source);
    expectToken( Tokenizer::Tokens::OP_COLON, source, tokensConsumed, "Expected \":\" after variable name", "Unexpected EOF" );
    tokensConsumed += type.parse( source.subslice(tokensConsumed) );

    RULE_LEAVE();
}

size_t VariableDefinition::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

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

    RULE_LEAVE();
}

size_t Statement::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);
    try {
        Expression expression;
        tokensConsumed = expression.parse(source);

        expectToken( Tokenizer::Tokens::SEMICOLON, source, tokensConsumed, "Statement does not end with a semicolon",
                "Unexpected EOF" );

        content = std::move(expression);

        RULE_LEAVE();
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
    }

    VariableDefinition def;

    tokensConsumed += def.parse(source);
    expectToken( Tokenizer::Tokens::SEMICOLON, source, tokensConsumed, "Statement does not end with a semicolon",
            "Unexpected EOF" );

    content = std::move(def);
    RULE_LEAVE();
}

size_t StatementList::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

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

    RULE_LEAVE();
}

size_t CompoundExpression::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

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

    RULE_LEAVE();
}

size_t FuncDeclRet::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

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

    RULE_LEAVE();
}

size_t FuncDeclArg::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    tokensConsumed += name.parse( source.subslice(tokensConsumed) );
    expectToken(
            Tokenizer::Tokens::OP_COLON, source, tokensConsumed, "Expected colon in argument declaration",
            "EOF while parsing function declaration" );
    tokensConsumed += type.parse( source.subslice(tokensConsumed) );

    RULE_LEAVE();
}

size_t FuncDeclArgsNonEmpty::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    bool done = false;
    do {
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

    RULE_LEAVE();
}

size_t FuncDeclArgs::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    try {
        FuncDeclArgsNonEmpty args;
        tokensConsumed += args.parse(source);
        arguments = std::move(args.arguments);
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
        // That didn't match - use the empty match rule
    }

    RULE_LEAVE();
}

size_t FuncDeclBody::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    tokensConsumed += name.parse( source  );

    expectToken( Tokenizer::Tokens::BRACKET_ROUND_OPEN, source, tokensConsumed, "Expected '('",
            "EOF while parsing function declaration" );

    tokensConsumed += arguments.parse( source.subslice(tokensConsumed) );

    expectToken( Tokenizer::Tokens::BRACKET_ROUND_CLOSE, source, tokensConsumed, "Expected ')'",
            "EOF while parsing function declaration" );

    tokensConsumed += returnType.parse( source.subslice(tokensConsumed) );

    RULE_LEAVE();
}

size_t FuncDef::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    const Tokenizer::Token *currentToken = nextToken(source, tokensConsumed, "EOF while looking for function definition");
    if( currentToken->token!=Tokenizer::Tokens::RESERVED_DEF ) {
        throw parser_error("Function definition should start with \"def\"", currentToken->line, currentToken->col);
    }

    tokensConsumed += decl.parse( source.subslice(tokensConsumed) );
    tokensConsumed += body.parse( source.subslice(tokensConsumed) );

    RULE_LEAVE();
}

void Module::parse(String source) {
    tokens = Tokenizer::Tokenizer::tokenize(source);
    parse(tokens);
}

size_t Module::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    while( tokensConsumed<source.size() ) {
        if( skipWS( source, tokensConsumed ) ) {
            continue;
        }

        FuncDef func;

        tokensConsumed += func.parse( source.subslice(tokensConsumed) );
        functionDefinitions.emplace_back( std::move(func) );
    }

    RULE_LEAVE();
}

} // namespace NonTerminals
