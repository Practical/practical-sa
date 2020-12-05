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
    auto overloadIterIter = funcDef->firstPassOverloads.find( parserFunction.decl.name.identifier );
    ASSERT( overloadIterIter != funcDef->firstPassOverloads.end() );
    ASSERT( overloadIterIter->second != funcDef->overloads.end() );

    functionType = overloadIterIter->second->second.type;
    mangledName = overloadIterIter->second->second.mangledName;

    auto funcType = overloadIterIter->second->first->getType();
    auto function = std::get_if< const StaticType::Function * >( &funcType );
    ASSERT( function!=nullptr );

    size_t numArguments = (*function)->getNumArguments();
    arguments.reserve( numArguments );
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
    }
}

void Function::codeGen( std::shared_ptr<FunctionGen> functionGen ) {
    functionGen->functionEnter(
            String(mangledName),
            getReturnType(),
            arguments,
            "",
            parserFunction.decl.name.identifier->location );

    struct Visitor {
        Function *_this;
        FunctionGen *functionGen;

        void operator()( const std::monostate &mono ) {
            ABORT()<<"Unreachable code reached";
        }

        void operator()( const NonTerminals::CompoundExpression &parserExpression ) {
            _this->codeGen( parserExpression.statementList, functionGen );

            Expression expression( parserExpression.expression );

            Weight weight;
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

    sl.buildAST( lookupCtx );
    sl.codeGen( lookupCtx, functionGen );
}

StaticTypeImpl::CPtr Function::getReturnType() const {
    auto parentPtr = std::get<const StaticType::Function *>( functionType->getType() )->getReturnType();
    return static_cast<const StaticTypeImpl *>( parentPtr.get() );
}

} // namespace AST
