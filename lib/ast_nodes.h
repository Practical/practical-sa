#ifndef AST_NODES_H
#define AST_NODES_H

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

    void symbolsPass2(LookupContext *ctx);

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
    ExpressionId codeGen(FunctionGen *codeGen, const StaticType *expectedResult);

private:
    void codeGenStatement(FunctionGen *codeGen, const NonTerminals::Statement *statement);
    ExpressionId codeGenExpression(
            FunctionGen *codeGen, const StaticType *expectedResult, const NonTerminals::Expression *expression);
    void codeGenVarDef(FunctionGen *codeGen, const NonTerminals::VariableDefinition *definition);
    ExpressionId codeGenLiteral(FunctionGen *codeGen, const StaticType *expectedResult, const NonTerminals::Literal *literal);
};

class FuncDeclArgs : NoCopy {
};

class FuncDecl : NoCopy {
    const NonTerminals::FuncDeclBody *parserFuncDecl;

    Type retType;
    std::vector<FuncDeclArgs> arguments;
public:
    FuncDecl(const NonTerminals::FuncDeclBody *nt);

    String getName() const {
        return parserFuncDecl->name.text;
    }

    size_t getLine() const {
        return parserFuncDecl->name.line;
    }

    size_t getCol() const {
        return parserFuncDecl->name.col;
    }

    const StaticType &getRetType() const {
        return retType.getType();
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
    FuncDef(const NonTerminals::FuncDef *nt, LookupContext *ctx);

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
