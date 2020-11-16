/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/expression/identifier.h"

#include <practical-errors.h>

using namespace PracticalSemanticAnalyzer;

namespace AST::ExpressionImpl {

Identifier::Identifier( const NonTerminals::Identifier &parserIdentifier ) :
    parserIdentifier( parserIdentifier )
{
}

String Identifier::getName() const {
    return parserIdentifier.identifier->text;
}

size_t Identifier::getLine() const {
    return parserIdentifier.identifier->line;
}

size_t Identifier::getCol() const {
    return parserIdentifier.identifier->col;
}

void Identifier::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit )
{
    identifier = lookupContext.lookupIdentifier( parserIdentifier.identifier->text );

    if( identifier==nullptr ) {
        throw SymbolNotFound(
                parserIdentifier.identifier->text, parserIdentifier.identifier->line, parserIdentifier.identifier->col );
    }

    struct Visitor {
        Identifier *_this;
        ExpectedResult &expectedResult;

        void operator()( const LookupContext::Variable &var ) {
            _this->metadata.type = downCast( var.type->addFlags( StaticType::Flags::Reference ) );
            _this->metadata.valueRange = var.type->defaultRange();
        }

        void operator()( const LookupContext::Function &func ) {
            ASSERT( !expectedResult )<<"TODO implement choosing overload based on expected result";
        }
    };

    std::visit( Visitor{._this=this, .expectedResult=expectedResult}, *identifier );
}

ExpressionId Identifier::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    struct Visitor {
        PracticalSemanticAnalyzer::FunctionGen *functionGen;

        ExpressionId operator()( const LookupContext::Variable &var ) {
            if( var.lvalueId!=ExpressionId() ) {
                return var.lvalueId;
            }

            ABORT()<<"TODO implement by name identifier lookup";
        }

        ExpressionId operator()( const LookupContext::Function &func ) {
            ABORT()<<"Code gen called for function name";
        }
    };

    return std::visit( Visitor{.functionGen = functionGen}, *identifier );
}

} // namespace AST::ExpressionImpl
