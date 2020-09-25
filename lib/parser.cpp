/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "parser.h"

#include "parser_internal.h"
#include "practical-errors.h"
#include "scope_tracing.h"

using namespace InternalNonTerminals;

namespace NonTerminals {

size_t Identifier::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    identifier = &expectToken(Tokenizer::Tokens::IDENTIFIER, source, tokensConsumed, "Expected an identifier",
            "EOF while parsing an identifier" );

    RULE_LEAVE();
}

size_t Type::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    Identifier &id = type.emplace<Identifier>();

    tokensConsumed = id.parse( source );

    bool done=false;

    do {
        size_t provisionalyConsumed = 0;
        const Tokenizer::Token *token = nextToken( source.subslice(tokensConsumed), provisionalyConsumed, nullptr );
        if( !token )
            break;

        switch( token->token ) {
        case Tokenizer::Tokens::OP_ASTERISK:
            {
                auto pointedType = std::make_unique<Type>();
                pointedType->type = std::move(type);

                type.emplace< Pointer >( std::move(pointedType), token );
            }
            break;
        default:
            done=true;
            break;
        }

        if( !done )
            tokensConsumed += provisionalyConsumed;
    } while(!done);

    RULE_LEAVE();
}

size_t Type::getLine() const {
    struct Visitor {
        size_t operator()( std::monostate ) {
            ABORT()<<"Unreachable state";
        }

        size_t operator()( const Identifier &id ) {
            return id.getLine();
        }

        size_t operator()( const Pointer &ptr ) {
            return ptr.token->line;
        }
    };

    return std::visit( Visitor{}, type );
}

size_t Type::getCol() const {
    struct Visitor {
        size_t operator()( std::monostate ) {
            ABORT()<<"Unreachable state";
        }

        size_t operator()( const Identifier &id ) {
            return id.getCol();
        }

        size_t operator()( const Pointer &ptr ) {
            return ptr.token->col;
        }
    };

    return std::visit( Visitor{}, type );
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
    case Tokenizer::Tokens::RESERVED_TRUE:
    case Tokenizer::Tokens::RESERVED_FALSE:
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

    if( wishForToken(Tokenizer::Tokens::RESERVED_IF, source, tokensConsumed) ) {
        ConditionalExpressionOrStatement condition;
        tokensConsumed = condition.parse( source, ExpectedResult::Expression );

        value = safenew<ConditionalExpression>( condition.removeExpression() );

        RULE_LEAVE();
    }

    tokensConsumed=0;
    if( wishForToken(Tokenizer::Tokens::BRACKET_CURLY_OPEN, source, tokensConsumed) ) {
        CompoundExpression compound;
        tokensConsumed=compound.parse(source);

        value = safenew<CompoundExpression>( std::move(compound) );

        RULE_LEAVE();
    }

    std::exception_ptr expressionParseError;
    try {
        tokensConsumed = actualParse(source, Operators::operators.size());
        RULE_LEAVE();
    } catch( parser_error &error ) {
        EXCEPTION_CAUGHT(error);
        expressionParseError = std::current_exception();
    }

    try {
        Type *type = &value.emplace<Type>();
        tokensConsumed = type->parse(source);
    } catch( parser_error &error ) {
        EXCEPTION_CAUGHT(error);
        // We only tried to parse as type as a hail Mary. If it failed, we want the original error
        std::rethrow_exception(expressionParseError);
    }

    RULE_LEAVE();
}

const Type *Expression::reparseAsType() const {
    const Type *ret = std::get_if<NonTerminals::Type>( &value );

    if( ret!=nullptr ) {
        return ret;
    }

    if( !altTypeParse ) {
        altTypeParse = safenew<NonTerminals::Type>();

        size_t tokensConsumed = altTypeParse->parse( getNTTokens() );
        if( tokensConsumed != getNTTokens().size() ) {
            ASSERT( tokensConsumed < getNTTokens().size() ) <<
                    "Undetected range error during parse: " << tokensConsumed << "<" << getNTTokens().size();
            const Tokenizer::Token *currentToken = &getNTTokens()[ tokensConsumed ];
            throw parser_error("Type parsing did not consume entire range", currentToken->line, currentToken->col);
        }
    }

    return altTypeParse.get();
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
        EXCEPTION_CAUGHT(err);
    }

    // Or maybe a Literal
    tokensConsumed = value.emplace<Literal>().parse(source);

    RULE_LEAVE();
}

size_t Expression::parsePrefixOp(
        Slice<const Tokenizer::Token> source, size_t level, const Operators::OperatorPriority::OperatorsMap &operators)
{
    RULE_ENTER(source);

    const Tokenizer::Token *op =nextToken( source, tokensConsumed, "End of file while looking for operator" );

    auto operatorInfo = operators.find( op->token );

    if( operatorInfo!=operators.end() ) {
        switch( operatorInfo->second ) {
        case Operators::OperatorType::Regular:
            {
                UnaryOperator &op1 = value.emplace< UnaryOperator >();
                op1.op = op;
                // The operator we found is a valid prefix operator for this level
                op1.operand = safenew< Expression >();
                tokensConsumed += op1.operand->parsePrefixOp( source.subslice(tokensConsumed), level, operators );
            }
            RULE_LEAVE();
        case Operators::OperatorType::Cast:
            {
                CastOperator &cast = value.emplace< CastOperator >();
                cast.op = op;
                expectToken(
                        Tokenizer::Tokens::OP_TEMPLATE_EXPAND,
                        source,
                        tokensConsumed,
                        "Cast operator must be followed by `!`",
                        "End of file looking for cast expression"
                );
                tokensConsumed += cast.destType.parse( source.subslice(tokensConsumed) );
                expectToken(
                        Tokenizer::Tokens::BRACKET_ROUND_OPEN,
                        source,
                        tokensConsumed,
                        "Expected `(` after cast type",
                        "End of file looking for cast expression"
                );
                cast.expression = safenew< Expression >();
                tokensConsumed += cast.expression->parse( source.subslice( tokensConsumed ) );
                expectToken(
                        Tokenizer::Tokens::BRACKET_ROUND_CLOSE,
                        source,
                        tokensConsumed,
                        "Expected ')'",
                        "End of file looking for terminating ')'"
                );
            }
            RULE_LEAVE();
        default:
            ;
        }
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
     * The direct translation to code of the above is to start by calling op.operands[0]->actualParse( source, level )
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
    op.operands[0] = safenew<Expression>();
    tokensConsumed = op.operands[0]->actualParse( source, level-1 );

    size_t provisionalTokensConsumed = tokensConsumed;
    op.op = nextToken( source, provisionalTokensConsumed );
    if( op.op==nullptr ) {
        *this = std::move(* op.operands[0]);
        RULE_LEAVE();
    }

    auto opInfo = operators.find(op.op->token);
    if( opInfo==operators.end() ) {
        // This is not the operator you're looking for. Just make do with what we have without it
        *this = std::move(* op.operands[0]);
        RULE_LEAVE();
    }

    ASSERT( opInfo->second==Operators::OperatorType::Regular );
    tokensConsumed = provisionalTokensConsumed;

    op.operands[1] = safenew< Expression >();
    tokensConsumed += op.operands[1]->actualParse( source.subslice(tokensConsumed), level-1 );

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
        op2.operands[0] = safenew< Expression >();
        op2.operands[0]->value = std::move( op );
        op = std::move( op2 );

        op.operands[1] = safenew< Expression >();
        tokensConsumed += op.operands[1]->actualParse( source.subslice( tokensConsumed ), level-1 );
    }

    value = std::move(op);

    RULE_LEAVE();
}

size_t Expression::parseInfixR2LOp(
        Slice<const Tokenizer::Token> source, size_t level, const Operators::OperatorPriority::OperatorsMap &operators)
{
    RULE_ENTER(source);

    BinaryOperator op;
    op.operands[0] = safenew< Expression >();
    tokensConsumed += op.operands[0]->actualParse( source, level-1 );

    size_t provisionalTokensConsumed = tokensConsumed;
    op.op = nextToken( source, provisionalTokensConsumed );
    if( op.op==nullptr ) {
        *this = std::move(* op.operands[0]);
        RULE_LEAVE();
    }

    auto opInfo = operators.find(op.op->token);
    if( opInfo==operators.end() ) {
        // This is not the operator you're looking for. Just make do with what we have without it
        *this = std::move(* op.operands[0]);
        RULE_LEAVE();
    }
    ASSERT( opInfo->second==Operators::OperatorType::Regular );

    tokensConsumed = provisionalTokensConsumed;

    op.operands[1] = safenew< Expression >();
    tokensConsumed += op.operands[1]->actualParse( source.subslice(tokensConsumed), level );

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
                funcCall.op = op.op;
                funcCall.expression = safenew< Expression >( std::move( *this ) );
                tokensConsumed += funcCall.arguments.parse( source.subslice(tokensConsumed) );
                value = std::move( funcCall );
            }
            break;
        case Operators::OperatorType::SliceSubscript:
            ABORT() << "TODO implement";
        case Operators::OperatorType::Cast:
            ABORT() << "Cast is not a postfix operator";
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

    ConditionalExpressionOrStatement condition;
    if( wishForToken(Tokenizer::Tokens::RESERVED_IF, source, tokensConsumed) ) {
        tokensConsumed = condition.parse(source);
        if( condition.isStatement() ) {
            content = condition.removeStatement();
        } else {
            auto expression = Expression( condition.removeExpression() );
            // XXX Don't handle conditional expression that is part of a larger expression
            //tokensConsumed += expression.continueParse( source.subslice(tokensConsumed) );

            expectToken( Tokenizer::Tokens::SEMICOLON, source, tokensConsumed, "Statement does not end with a semicolon",
                    "Unexpected EOF" );

            content = std::move(expression);
        }

        RULE_LEAVE();
    }

    tokensConsumed=0;
    if( wishForToken(Tokenizer::Tokens::BRACKET_CURLY_OPEN, source, tokensConsumed) ) {
        CompoundStatement compound;
        tokensConsumed=compound.parse(source);

        content = safenew<CompoundStatement>( std::move(compound) );

        RULE_LEAVE();
    }

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

    CompoundExpressionOrStatement compound;
    tokensConsumed = compound.parseExpression(source);

    *this = compound.removeExpression();

    RULE_LEAVE();
}

size_t CompoundStatement::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    CompoundExpressionOrStatement compound;
    tokensConsumed = compound.parseStatement(source);

    *this = compound.removeStatement();

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

    bool more = false;
    do {
        FuncDeclArg arg;
        tokensConsumed += arg.parse(source.subslice(tokensConsumed));
        arguments.emplace_back( std::move(arg) );

        more = wishForToken( Tokenizer::Tokens::COMMA, source, tokensConsumed, true );
    } while( more );

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
    CompoundExpressionOrStatement body;
    tokensConsumed += body.parse( source.subslice(tokensConsumed) );
    if( body.isStatement() )
        this->body = body.removeStatement();
    else
        this->body = body.removeExpression();

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
