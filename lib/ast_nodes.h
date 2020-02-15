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
#include "expression.h"
#include "lookup_context.h"
#include "parser.h"

#include <deque>

namespace AST {

class Type : NoCopy {
    const NonTerminals::Type *parseType = nullptr;
    StaticType::Ptr staticType;

public:
    explicit Type(const NonTerminals::Type *nt);
    explicit Type(const NonTerminals::Expression *nt);

    StaticType::Ptr getType() const {
        return staticType;
    }

    void symbolsPass2(const LookupContext *ctx);

    StaticType::Ptr removeType() && {
        return std::move(staticType);
    }
};

class Statement : NoCopy {
    LookupContext &ctx;
    const NonTerminals::Statement &parserStatement;
public:
    Statement(LookupContext &ctx, const NonTerminals::Statement &nt);

    void codeGen(FunctionGen *codeGen);

private:
    void codeGenVarDef(FunctionGen *codeGen, const NonTerminals::VariableDefinition &definition);
};

class CompoundStatement : NoCopy {
    LookupContext ctx;
    const NonTerminals::CompoundStatement *parserStatement;

public:
    CompoundStatement(LookupContext *parentCtx, const NonTerminals::CompoundStatement *nt);

    void symbolsPass1();
    void symbolsPass2();
    void codeGen(FunctionGen *codeGen);
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

    StaticType::Ptr getRetType() const {
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
    std::variant<std::monostate, CompoundExpression, CompoundStatement> body;
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
