#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <practical-sa.h>

#include "expected_type.h"
#include "lookup_context.h"
#include "parser.h"

using namespace PracticalSemanticAnalyzer;

namespace CodeGen {

ExpectedType findCommonType(
        const Tokenizer::Token *op, LookupContext &ctx, const Expression &expr1, const Expression &expr2 );

Expression codeGenExpression(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType, const NonTerminals::Expression *parserExpression);
void codeGenStatement(LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::Statement *parserStatement);

Expression codeGenLiteral(FunctionGen *codeGen, ExpectedType expectedResult, const NonTerminals::Literal *literal);
Expression codeGenIdentifierLookup(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult, const Tokenizer::Token *identifier);
Expression codeGenFunctionCall(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::FunctionCall *functionCall);

void codeGenVarDef(LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::VariableDefinition *definition);
void codeGenCondition(
        LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::Statement::ConditionalStatement *condition);
Expression codeGenCondition(
        LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::ConditionalExpression *condition,
        ExpectedType expectedResult);
} // End namespace CodeGen

#endif // CODE_GEN_H
