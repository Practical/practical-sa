/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/statement.h"

#include "ast/compound_statement.h"
#include "ast/expression.h"

namespace AST {

Statement::Statement( const NonTerminals::Statement &parserStatement ) : parserStatement(parserStatement)
{
}

Statement::Statement( const Statement &that ) :
    parserStatement( *(const NonTerminals::Statement *)(1) )
{
    ABORT()<<"ASSERT FAILED: Statement copy constructor called";
}

Statement::~Statement() {
    // The only use for the explicit definition of the destructor is to postpone the definition after we can #include
    // CompoundStatement.
}

void Statement::buildAST( LookupContext &lookupCtx ) {
    struct Visitor {
        Statement &_this;
        LookupContext &lookupCtx;

        void operator()( std::monostate mono ) {
            ABORT()<<"Statement is in monostate";
        }

        void operator()( const NonTerminals::Expression &parserExpression ) {
            auto &expression = _this.underlyingStatement.emplace<Expression>(parserExpression);
            unsigned weight=0;
            expression.buildAST(lookupCtx, ExpectedResult(), weight, Expression::NoWeightLimit);
        }

        void operator()( const NonTerminals::VariableDefinition &parserVarDef ) {
            auto &varDef = _this.underlyingStatement.emplace<VariableDefinition>(parserVarDef);
            varDef.buildAST(lookupCtx);
        }

        void operator()( const NonTerminals::Statement::ConditionalStatement &parserCondition ) {
            auto &condition = _this.underlyingStatement.emplace<ConditionalStatement>(parserCondition);
            condition.buildAST(lookupCtx);
        }

        void operator()( const std::unique_ptr<NonTerminals::CompoundStatement> &parserCompound ) {
            auto &compound = _this.underlyingStatement.emplace<
                    std::unique_ptr<CompoundStatement>
                >(
                    safenew<CompoundStatement>( *parserCompound, lookupCtx )
                );
            compound->buildAST();
        }
    };

    std::visit( Visitor{ ._this=*this, .lookupCtx=lookupCtx }, parserStatement.content );
}

void Statement::codeGen( const LookupContext &lookupCtx, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    struct Visitor {
        const LookupContext &lookupCtx;
        PracticalSemanticAnalyzer::FunctionGen *functionGen;

        void operator()( std::monostate mono ) {
            ABORT()<<"Statement is in monostate";
        }

        void operator()( const Expression &expression ) {
            expression.codeGen(functionGen);
        }

        void operator()( const VariableDefinition &varDef ) {
            varDef.codeGen( lookupCtx, functionGen );
        }

        void operator()( const NonTerminals::Statement::ConditionalStatement &condition ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const ConditionalStatement &condition ) {
            condition.codeGen(lookupCtx, functionGen);
        }

        void operator()( const std::unique_ptr<CompoundStatement> &compound ) {
            compound->codeGen(functionGen);
        }
    };

    std::visit( Visitor{ .lookupCtx=lookupCtx, .functionGen=functionGen}, underlyingStatement );
}

} // namespace AST
