/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/variable_definition.h"

using namespace PracticalSemanticAnalyzer;

namespace AST {

VariableDefinition::VariableDefinition(const NonTerminals::VariableDefinition &parserVarDef) :
    parserVarDef( parserVarDef )
{
    if( parserVarDef.initValue )
        initValue.emplace( *parserVarDef.initValue );
}

void VariableDefinition::buildAST( LookupContext &lookupCtx ) {
    auto varType = lookupCtx.lookupType( parserVarDef.body.type );

    if( initValue ) {
        Weight weight;
        initValue->buildAST(lookupCtx, varType, weight, Expression::NoWeightLimit);
    }

    lookupCtx.addLocalVar( parserVarDef.body.name.identifier, varType, Expression::allocateId() );
}

void VariableDefinition::codeGen(
        const LookupContext &lookupCtx, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const
{
    const LookupContext::Identifier *identifier = lookupCtx.lookupIdentifier( parserVarDef.body.name.identifier->text );
    const auto &varDef = std::get< LookupContext::Variable >(*identifier);

    functionGen->allocateStackVar(varDef.lvalueId, varDef.type, parserVarDef.body.name.identifier->text);

    if( initValue ) {
        ExpressionId initValueExpressionId = initValue->codeGen(functionGen);

        functionGen->assign( varDef.lvalueId, initValueExpressionId );
    }
}

} // namespace AST
