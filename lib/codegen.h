#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <practical-sa.h>

#include "expected_type.h"
#include "lookup_context.h"
#include "parser.h"

using namespace PracticalSemanticAnalyzer;

namespace CodeGen {

ExpectedType findCommonType(
        const Tokenizer::Token *op, LookupContext &ctx, const AST::Expression &expr1, const AST::Expression &expr2 );

AST::Expression codeGenExpression(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType, const NonTerminals::Expression *parserExpression);
void codeGenStatement(LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::Statement *parserStatement);

AST::Expression codeGenLiteral(FunctionGen *codeGen, ExpectedType expectedResult, const NonTerminals::Literal *literal);
AST::Expression codeGenIdentifierLookup(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult, const Tokenizer::Token *identifier);
AST::Expression codeGenFunctionCall(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::FunctionCall *functionCall);

void codeGenVarDef(LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::VariableDefinition *definition);
void codeGenCondition(
        LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::Statement::ConditionalStatement *condition);
AST::Expression codeGenCondition(
        LookupContext &ctx, FunctionGen *codeGen, const NonTerminals::ConditionalExpression *condition,
        ExpectedType expectedResult);
} // End namespace CodeGen

#endif // CODE_GEN_H
