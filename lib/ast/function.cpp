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
#include "ast/statement_list.h"

using namespace PracticalSemanticAnalyzer;

namespace AST {

Function::Function( const NonTerminals::FuncDef &parserFunction, const LookupContext &parentCtx ) :
    parserFunction( parserFunction ),
    name( parserFunction.decl.name.identifier->text ),
    lookupCtx( parentCtx )
{
    const LookupContext::Symbol *funcType = parentCtx.lookupSymbol( name );
    ASSERT( funcType );
    StaticType::Types type = funcType->type->getType();
    auto function = std::get_if< const StaticType::Function * >(&type);
    ASSERT( function!=nullptr );

    returnType = (*function)->getReturnType();
    size_t numArguments = (*function)->getNumArguments();
    arguments.reserve( numArguments );
    for( unsigned i=0; i<numArguments; ++i ) {
        arguments.emplace_back(
                (*function)->getArgumentType( i ),
                parserFunction.decl.arguments.arguments[i].name.identifier->text,
                Expression::allocateId()
        );
    }
}

void Function::codeGen( std::shared_ptr<FunctionGen> functionGen ) {
    functionGen->functionEnter(
            name,
            returnType,
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

            expression.buildAST( _this->lookupCtx, _this->returnType );
            expression.codeGen( functionGen );

            functionGen->returnValue( expression.getId() );
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

} // namespace AST
