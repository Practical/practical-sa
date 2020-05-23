/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "function.h"

#include "ast/ast.h"
#include "ast/expression.h"
#include "ast/mangle.h"
#include "ast/statement_list.h"

using namespace PracticalSemanticAnalyzer;

namespace AST {

Function::Function( const NonTerminals::FuncDef &parserFunction, const LookupContext &parentCtx ) :
    parserFunction( parserFunction ),
    name( parserFunction.decl.name.identifier->text ),
    lookupCtx( &parentCtx )
{
    const LookupContext::Identifier *identifierDef = parentCtx.lookupIdentifier( name );
    ASSERT( identifierDef );
    const LookupContext::Function *funcDef = std::get_if<LookupContext::Function>( identifierDef );
    ASSERT( funcDef );
    auto overload = funcDef->overloads.find( parserFunction.decl.name.identifier );
    ASSERT( overload!=funcDef->overloads.end() );
    auto funcType = overload->second.type->getType();
    auto function = std::get_if< const StaticType::Function * >( &funcType );
    ASSERT( function!=nullptr );

    StaticTypeImpl::CPtr returnType = static_cast<const StaticTypeImpl *>( (*function)->getReturnType().get() );
    std::vector<StaticTypeImpl::CPtr> argumentTypes;

    size_t numArguments = (*function)->getNumArguments();
    arguments.reserve( numArguments );
    argumentTypes.reserve( numArguments );
    for( unsigned i=0; i<numArguments; ++i ) {
        ExpressionId varExpressionId = Expression::allocateId();
        StaticTypeImpl::CPtr argumentType = static_cast<const StaticTypeImpl *>( (*function)->getArgumentType(i).get() );
        lookupCtx.addLocalVar(
                parserFunction.decl.arguments.arguments[i].name.identifier,
                argumentType,
                varExpressionId );
        arguments.emplace_back(
                (*function)->getArgumentType( i ),
                parserFunction.decl.arguments.arguments[i].name.identifier->text,
                varExpressionId
        );
        argumentTypes.emplace_back( argumentType );
    }

    functionType = StaticTypeImpl::allocate( FunctionTypeImpl( std::move(returnType), std::move(argumentTypes) ) );
    mangledName = getFunctionMangledName( name, functionType );
}

void Function::codeGen( std::shared_ptr<FunctionGen> functionGen ) {
    functionGen->functionEnter(
            String(mangledName),
            getReturnType(),
            arguments,
            "",
            parserFunction.decl.name.identifier->line,
            parserFunction.decl.name.identifier->col );

    struct Visitor {
        Function *_this;
        FunctionGen *functionGen;

        void operator()( const std::monostate &mono ) {
            ABORT()<<"Unreachable code reached";
        }

        void operator()( const NonTerminals::CompoundExpression &parserExpression ) {
            _this->codeGen( parserExpression.statementList, functionGen );

            Expression expression( parserExpression.expression );

            unsigned weight = 0;
            expression.buildAST( _this->lookupCtx, _this->getReturnType(), weight, Expression::NoWeightLimit );

            functionGen->returnValue( expression.codeGen( functionGen ) );
        }

        void operator()( const NonTerminals::CompoundStatement &parserStatement ) {
            _this->codeGen( parserStatement.statements, functionGen );

            functionGen->returnValue();
        }
    };

    std::visit( Visitor{ ._this = this, .functionGen = functionGen.get() }, parserFunction.body );

    functionGen->functionLeave();
}

void Function::codeGen(
        const NonTerminals::StatementList &statementList, PracticalSemanticAnalyzer::FunctionGen *functionGen )
{
    StatementList sl( statementList );

    sl.codeGen( lookupCtx, functionGen );
}

StaticTypeImpl::CPtr Function::getReturnType() const {
    auto parentPtr = std::get<const StaticType::Function *>( functionType->getType() )->getReturnType();
    return static_cast<const StaticTypeImpl *>( parentPtr.get() );
}

} // namespace AST
