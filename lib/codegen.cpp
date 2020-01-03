#include "codegen.h"

#include "ast.h"
#include "casts.h"
#include "codegen_operators.h"
#include "dummy_codegen_impl.h"

#include <practical-errors.h>

using namespace PracticalSemanticAnalyzer;

namespace CodeGen {

static JumpPointId::Allocator<> jumpPointAllocator;

static StaticType::Ptr findCommonType(
        const PracticalSemanticAnalyzer::NamedType *type1,
        const PracticalSemanticAnalyzer::NamedType *type2 )
{
    if( type1==nullptr || type2==nullptr ) {
        return StaticType::Ptr();
    }

    if( type2->type()==NamedType::Type::UnsignedInteger ) {
        // Make sure that if either types is UnsignedInteger, that type1 is UnsignedInteger
        std::swap( type2, type1 );
    }

    if( type1->type()==NamedType::Type::UnsignedInteger ) {
        if( type2->type()==NamedType::Type::UnsignedInteger ) {
            return StaticType::allocate( (type1->size()>type2->size() ? type1 : type2)->id() );
        } else if( type2->type()==NamedType::Type::SignedInteger ) {
            // Unsigned and signed. This might get hairy
            if( type2->size() > type1->size() ) {
                return StaticType::allocate( type2->id() );
            }

            char retTypeName[10];
            snprintf(retTypeName, sizeof(retTypeName), "S%lu", type1->size()*2);
            auto retType = AST::AST::getGlobalCtx().lookupType(retTypeName);
            if( retType != nullptr ) {
                return StaticType::allocate( retType->id() );
            }
        }
    } else if( type1->type()==NamedType::Type::SignedInteger ) {
        if( type2->type()!=NamedType::Type::SignedInteger )
            return StaticType::Ptr();

        return StaticType::allocate( (type1->size()>type2->size() ? type1 : type2)->id() );
    }

    return StaticType::Ptr();
}

ExpectedType findCommonType(
        const Tokenizer::Token *op, LookupContext &ctx, const Expression &expr1, const Expression &expr2 )
{
    if( expr1.type == expr2.type )
        return ExpectedType( expr1.type );

    using PracticalSemanticAnalyzer::NamedType;
    auto type1 = ctx.lookupType( expr1.type->getId() );
    auto type2 = ctx.lookupType( expr2.type->getId() );

    auto retType = findCommonType( type1, type2 );
    if( !retType )
        throw PracticalSemanticAnalyzer::IncompatibleTypes( expr1.type, expr2.type, op->line, op->col );

    return ExpectedType( retType );
}

void codeGenStatement(LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::Statement *statement) {

    struct Visitor {
        LookupContext &ctx;
        FunctionGen *codeGen;

        void operator()(const NonTerminals::Expression &expression) {
            codeGenExpression(ctx, codeGen, ExpectedType(), &expression); // Disregard return value
        }

        void operator()(const NonTerminals::VariableDefinition &definition) {
            codeGenVarDef(ctx, codeGen, &definition);
        }

        void operator()(std::monostate mono) {
            ABORT() << "Codegen called on unparsed expression";
        }

        void operator()(const NonTerminals::Statement::ConditionalStatement &condition) {
            codeGenCondition(ctx, codeGen, &condition);
        }

        void operator()(const std::unique_ptr<NonTerminals::CompoundStatement> &statement) {
            AST::CompoundStatement astStatement(&ctx, statement.get());
            astStatement.symbolsPass1();
            astStatement.symbolsPass2();
            astStatement.codeGen(codeGen);
        }
    } visitor = { ctx, codeGen };

    std::visit( visitor, statement->content );
}

Expression codeGenExpression(
        LookupContext &ctx,
        FunctionGen *codeGen,
        ExpectedType expectedResult,
        const NonTerminals::Expression *expression)
{
    using namespace NonTerminals;

    struct Visitor {
        LookupContext &ctx;
        FunctionGen *codeGen;
        ExpectedType expectedResult;

        ::Expression operator()( std::monostate none ) const {
            return ::Expression();
        }

        ::Expression operator()( const std::unique_ptr<NonTerminals::CompoundExpression> &compound ) const {
            AST::CompoundExpression expression(&ctx, compound.get());
            expression.symbolsPass1();
            expression.symbolsPass2();
            return expression.codeGen(codeGen, expectedResult);
        }

        ::Expression operator()( const NonTerminals::Literal &literal ) const {
            return codeGenLiteral( codeGen, expectedResult, &literal );
        }

        ::Expression operator()( const NonTerminals::Identifier &identifier ) const {
            return codeGenIdentifierLookup(ctx, codeGen, expectedResult, identifier.identifier);
        }

        ::Expression operator()( const NonTerminals::Expression::UnaryOperator &op ) const {
            return codeGenUnaryOperator( ctx, codeGen, expectedResult, op );
        }

        ::Expression operator()( const NonTerminals::Expression::BinaryOperator &op ) const {
            return codeGenBinaryOperator( ctx, codeGen, expectedResult, op );
        }

        ::Expression operator()( const NonTerminals::Expression::FunctionCall &functionCall ) const {
            return codeGenFunctionCall(ctx, codeGen, expectedResult, &functionCall);
        }

        ::Expression operator()( const std::unique_ptr< NonTerminals::ConditionalExpression > &condition ) const {
            return codeGenCondition(ctx, codeGen, condition.get(), expectedResult);
        }

        ::Expression operator()( const NonTerminals::Type &parseType ) const {
            ABORT()<<"TODO untested code";
            auto result = ::Expression( StaticType::Ptr(typeType) );
            ::AST::Type semanticType( &parseType );

            result.compileTimeValue = semanticType.getType();

            return result;
        }
    };

    Visitor visitor = { ctx, codeGen, expectedResult };
    return std::visit( visitor, expression->value );
}

void codeGenVarDef(LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::VariableDefinition *definition) {
    AST::Type varType( &definition->body.type );
    varType.symbolsPass2( &ctx );
    const LookupContext::LocalVariable *namedVar = ctx.registerVariable( LookupContext::LocalVariable(
                definition->body.name.identifier, Expression( std::move(varType).removeType() ) ) );
    codeGen->allocateStackVar( namedVar->lvalueExpression.id, namedVar->lvalueExpression.type, namedVar->name->text );

    if( definition->initValue ) {
        Expression initValue = codeGenExpression(
                ctx, codeGen, ExpectedType( namedVar->lvalueExpression.type ), definition->initValue.get());
        codeGen->assign( namedVar->lvalueExpression.id, initValue.id );
    } else {
        ABORT() << "TODO Type default values not yet implemented";
    }
}

void codeGenCondition(
        LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::Statement::ConditionalStatement *condition)
{
    auto boolNamedType = AST::AST::getGlobalCtx().lookupType("Bool");
    StaticType::Ptr boolType = StaticType::allocate( boolNamedType->id() );
    Expression conditionExpression = codeGenExpression( ctx, codeGen, ExpectedType(boolType), &condition->condition );

    ASSERT( condition->ifClause )<<"Condition with no parsed \"if\" clause";

    // TODO Use VRP to only generate the side of the condition actually taken
    JumpPointId elseJumpPoint, contJumpPoint{ jumpPointAllocator.allocate() };
    if( condition->elseClause ) {
        elseJumpPoint = jumpPointAllocator.allocate();
    }
    codeGen->conditionalBranch(ExpressionId(), StaticType::Ptr(), conditionExpression.id, elseJumpPoint, contJumpPoint);

    codeGenStatement( ctx, codeGen, condition->ifClause.get() );

    if( condition->elseClause ) {
        codeGen->setJumpPoint( elseJumpPoint );
        codeGenStatement( ctx, codeGen, condition->elseClause.get() );
    }

    codeGen->setJumpPoint( contJumpPoint );
}

Expression codeGenCondition(
        LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::ConditionalExpression *condition,
        ExpectedType expectedResult)
{
    auto boolNamedType = AST::AST::getGlobalCtx().lookupType("Bool");
    StaticType::Ptr boolType = StaticType::allocate( boolNamedType->id() );
    Expression conditionExpression = codeGenExpression( ctx, codeGen, ExpectedType(boolType), &condition->condition );

    // TODO Use VRP to only generate the side of the condition actually taken
    JumpPointId elseJumpPoint{ jumpPointAllocator.allocate() }, contJumpPoint{ jumpPointAllocator.allocate() };

    if( !expectedResult.mandatory ) {
        Expression ifResult = codeGenExpression( ctx, &dummyFunctionGen, expectedResult, &condition->ifClause );
        Expression elseResult = codeGenExpression( ctx, &dummyFunctionGen, expectedResult, &condition->elseClause );

        Tokenizer::Token emptyTokenXXX;
        expectedResult = findCommonType( &emptyTokenXXX, ctx, ifResult, elseResult );
    }

    Expression result( StaticType::Ptr(expectedResult.type) );
    codeGen->conditionalBranch(result.id, expectedResult.type, conditionExpression.id, elseJumpPoint, contJumpPoint);

    Expression clauseResult = codeGenExpression( ctx, codeGen, expectedResult, &condition->ifClause );
    codeGen->setConditionClauseResult( clauseResult.id );

    codeGen->setJumpPoint( elseJumpPoint );
    clauseResult = codeGenExpression( ctx, codeGen, expectedResult, &condition->elseClause );
    codeGen->setConditionClauseResult( clauseResult.id );

    codeGen->setJumpPoint( contJumpPoint );

    // TODO Need to calculate unified VRP
    return result;
}

Expression codeGenLiteralInt(
        FunctionGen *codeGen, ExpectedType expectedResult, const NonTerminals::Literal *literal)
{
    String text = literal->token.text;

    LongEnoughInt resultValue = 0;

    switch( literal->token.token ) {
    case Tokenizer::Tokens::LITERAL_INT_10:
        for( char c: text ) {
            // TODO check range and assign type
            if( c=='_' )
                continue;

            ASSERT( c>='0' && c<='9' ) << "Decimal literal has character '"<<c<<"' out of allowed range";
            resultValue *= 10;
            resultValue += c-'0';
        }
        break;
    default:
        ABORT() << "TODO implement";
    }

    Expression result( AST::AST::deductLiteralRange(resultValue) );
    result.valueRange = ValueRange::allocate( resultValue, resultValue );
    if( expectedResult && checkImplicitCastAllowed(result, expectedResult, literal->token) ) {
        result.type = expectedResult.type;
    }

    codeGen->setLiteral(result.id, resultValue, result.type);

    return Expression( std::move(result) );
}

Expression codeGenLiteralBool(
        FunctionGen *codeGen, ExpectedType expectedResult, const NonTerminals::Literal *literal, bool value)
{
    auto boolType = AST::AST::getGlobalCtx().lookupType("Bool");
    Expression result( StaticType::allocate(boolType->id()) );

    codeGen->setLiteral(result.id, value);

    return Expression( std::move(result) );
}

Expression codeGenLiteral(FunctionGen *codeGen, ExpectedType expectedResult, const NonTerminals::Literal *literal)
{
    Expression expression;

    switch( literal->token.token ) {
    case Tokenizer::Tokens::LITERAL_INT_10:
    case Tokenizer::Tokens::LITERAL_INT_16:
    case Tokenizer::Tokens::LITERAL_INT_2:
        expression = codeGenLiteralInt(codeGen, expectedResult, literal);
        break;
    case Tokenizer::Tokens::RESERVED_TRUE:
        expression = codeGenLiteralBool(codeGen, expectedResult, literal, true);
        break;
    case Tokenizer::Tokens::RESERVED_FALSE:
        expression = codeGenLiteralBool(codeGen, expectedResult, literal, false);
        break;
    default:
        ABORT() << "TODO implement";
    }

    if( expectedResult )
        expression = codeGenCast( codeGen, expression, expectedResult, literal->token, true );

    return expression;
}

Expression codeGenIdentifierLookup(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult, const Tokenizer::Token *identifier)
{
    const LookupContext::NamedObject *referencedObject = ctx.lookupIdentifier( identifier->text );
    if( referencedObject==nullptr )
        throw SymbolNotFound(identifier->text, identifier->line, identifier->col);

    struct Visitor {
        const Tokenizer::Token *identifier;
        FunctionGen *codeGen;
        ExpectedType expectedResult;

        Expression operator()(const LookupContext::LocalVariable &localVar) const {
            Expression expression( StaticType::Ptr(localVar.lvalueExpression.type) );
            codeGen->dereferencePointer( expression.id, localVar.lvalueExpression.type, localVar.lvalueExpression.id );

            if( expectedResult )
                expression = codeGenCast( codeGen, expression, expectedResult, *identifier, true );

            return expression;
        }

        Expression operator()(const LookupContext::Function &function) const {
            throw CannotTakeValueOfFunction(identifier);
        }
    };

    return std::visit(
            Visitor{ .identifier=identifier, .codeGen=codeGen, .expectedResult=expectedResult }, *referencedObject );
}

Expression codeGenFunctionCall(
        LookupContext &ctx,
        FunctionGen *codeGen,
        ExpectedType expectedResult,
        const NonTerminals::Expression::FunctionCall *functionCall)
{
    const NonTerminals::Identifier *functionName = std::get_if<NonTerminals::Identifier>( & functionCall->expression->value );
    ASSERT( functionName != nullptr ) << "TODO support function calls not by means of direct call";

    const LookupContext::NamedObject *functionObject = ctx.lookupIdentifier( functionName->identifier->text );
    if( functionObject==nullptr ) {
        throw SymbolNotFound(
                functionName->identifier->text, functionName->identifier->line, functionName->identifier->col);
    }

    const LookupContext::Function *function = std::get_if< LookupContext::Function >( functionObject );
    if( function==nullptr ) {
        throw TryToCallNonCallable( functionName->identifier );
    }

    std::vector<ExpressionId> arguments;
    arguments.reserve( function->arguments.size() );

    for( unsigned int argNum=0; argNum<function->arguments.size(); ++argNum ) {
        Expression argument = codeGenExpression(
                ctx, codeGen, ExpectedType( function->arguments[argNum].type ), &functionCall->arguments.arguments[argNum] );
        ASSERT( argument.type == function->arguments[argNum].type ) <<
                "Expression " << argument << " did not return mandatory expected type " <<
                function->arguments[argNum].type;
        arguments.emplace_back( argument.id );
    }

    Expression functionRet( StaticType::Ptr(function->returnType) );
    codeGen->callFunctionDirect(
            functionRet.id, functionName->identifier->text, Slice<const ExpressionId>(arguments), function->returnType );

    if( !expectedResult )
        return functionRet;

    return codeGenCast( codeGen, functionRet, expectedResult, *functionName->identifier, true );
}

} // End namespace CodeGen
