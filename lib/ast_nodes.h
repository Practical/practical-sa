/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_NODES_H
#define AST_NODES_H

#include "expected_type.h"
#include "parser.h"

#include <deque>

namespace AST {

class Type : NoCopy {
    const NonTerminals::Type *parseType;
    StaticType staticType;

public:
    Type(const NonTerminals::Type *nt);

    const StaticType &getType() const {
        return staticType;
    }

    void symbolsPass2(const LookupContext *ctx);

    StaticType removeType() && {
        return std::move(staticType);
    }
};

class CompoundExpression : NoCopy {
    LookupContext ctx;
    const NonTerminals::CompoundExpression *parserExpression;

public:
    CompoundExpression(LookupContext *parentCtx, const NonTerminals::CompoundExpression *_parserExpression);

    void symbolsPass1();
    void symbolsPass2();
    ExpressionId codeGen(FunctionGen *codeGen, ExpectedType expectedResult);

private:
    void codeGenStatement(FunctionGen *codeGen, const NonTerminals::Statement *statement);
    ExpressionId codeGenExpression(
            FunctionGen *codeGen, ExpectedType expectedResult, const NonTerminals::Expression *expression);
    void codeGenVarDef(FunctionGen *codeGen, const NonTerminals::VariableDefinition *definition);
    ExpressionId codeGenLiteral(FunctionGen *codeGen, ExpectedType expectedResult, const NonTerminals::Literal *literal);
    ExpressionId codeGenIdentifierLookup(
            FunctionGen *codeGen, ExpectedType expectedResult, const Tokenizer::Token *identifier);
    ExpressionId codeGenUnaryOperator(
            FunctionGen *codeGen, ExpectedType expectedResult, const NonTerminals::Expression::UnaryOperator &op);
    ExpressionId codeGenBinaryOperator(
            FunctionGen *codeGen, ExpectedType expectedResult, const NonTerminals::Expression::BinaryOperator &op);
    ExpressionId codeGenFunctionCall(
            FunctionGen *codeGen, ExpectedType expectedResult, const NonTerminals::Expression::FunctionCall *functionCall);

};

class FuncDecl : NoCopy {
    const NonTerminals::FuncDeclBody *parserFuncDecl;
    LookupContext::Function *ctxFunction;
public:
    FuncDecl(const NonTerminals::FuncDeclBody *nt, LookupContext::Function *function);

    String getName() const {
        return parserFuncDecl->name.getName();
    }

    size_t getLine() const {
        return parserFuncDecl->name.getLine();
    }

    size_t getCol() const {
        return parserFuncDecl->name.getCol();
    }

    const StaticType &getRetType() const {
        return ctxFunction->returnType;
    }

    Slice<const ArgumentDeclaration> getArguments() const {
        return Slice<const ArgumentDeclaration>( ctxFunction->arguments );
    }

    void symbolsPass1(LookupContext *ctx);
    void symbolsPass2(LookupContext *ctx);
};

class FuncDef : NoCopy {
    const NonTerminals::FuncDef *parserFuncDef;
    PracticalSemanticAnalyzer::IdentifierId id;

    FuncDecl declaration;
    CompoundExpression body;
public:
    FuncDef(const NonTerminals::FuncDef *nt, LookupContext *ctx, LookupContext::Function *ctxFunction);

    String getName() const;
    IdentifierId getId() const;
    void setId(IdentifierId id);

    void symbolsPass1(LookupContext *ctx);
    void symbolsPass2(LookupContext *ctx);
    void codeGen(PracticalSemanticAnalyzer::ModuleGen *codeGenCB);
};

class Module : NoCopy {
    NonTerminals::Module *parseModule;
    LookupContext ctx;
    PracticalSemanticAnalyzer::ModuleId id;

    std::deque<FuncDef> functionDefinitions;
public:
    Module( NonTerminals::Module *parseModule, LookupContext *parentSymbolsTable );

    void symbolsPass1();
    void symbolsPass2();
    void codeGen(PracticalSemanticAnalyzer::ModuleGen *codeGenCB);
};

} // Namespace AST

#endif // AST_NODES_H
