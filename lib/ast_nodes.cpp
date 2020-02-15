/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast_nodes.h"

#include "ast.h"
#include "codegen.h"
#include "practical-errors.h"

#include "asserts.h"

#include <variant>

using PracticalSemanticAnalyzer::CannotTakeValueOfFunction;
using namespace CodeGen;

namespace AST {

Type::Type(const NonTerminals::Type *nt) : parseType(nt) {
}

Type::Type(const NonTerminals::Expression *nt) : parseType( nt->reparseAsType() ) {
}

void Type::symbolsPass2(const LookupContext *ctx) {
    auto type = ctx->lookupType(parseType->type.getName());
    if( type==nullptr )
        throw pass2_error("Type name expected", parseType->getLine(), parseType->getCol());

    ASSERT( !staticType );
    staticType = StaticType::allocate( type->id() );
}

CompoundExpression::CompoundExpression(LookupContext *parentCtx, const NonTerminals::CompoundExpression *_parserExpression)
    : ctx(parentCtx), parserExpression(_parserExpression)
{
}

void CompoundExpression::symbolsPass1() {
    // TODO implement
}

void CompoundExpression::symbolsPass2() {
    // TODO implement
}

Statement::Statement(LookupContext &ctx, const NonTerminals::Statement &nt) :
    ctx(ctx),
    parserStatement(nt)
{
}

void Statement::codeGen(FunctionGen *codeGen) {
    struct Visitor {
        Statement *_this;
        FunctionGen *codeGen;

        void operator()( std::monostate none ) {
            ABORT()<<"XXX Possibly unreachable state";
        }

        void operator()( const NonTerminals::Expression &parserExpression ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const NonTerminals::VariableDefinition &definition ) {
            _this->codeGenVarDef(codeGen, definition);
        }

        void operator()( const NonTerminals::Statement::ConditionalStatement &condition ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const std::unique_ptr<NonTerminals::CompoundStatement> &compound ) {
            ABORT()<<"TODO implement";
        }
    };

    std::visit( Visitor{ ._this = this, .codeGen = codeGen }, parserStatement.content );
}

void Statement::codeGenVarDef(FunctionGen *codeGen, const NonTerminals::VariableDefinition &definition) {
    Type varType( &definition.body.type );
    varType.symbolsPass2( &ctx );
    const LookupContext::LocalVariable *namedVar = ctx.registerVariable( LookupContext::LocalVariable(
                definition.body.name.identifier, Expression( std::move(varType).removeType() ) ) );
    codeGen->allocateStackVar(
            namedVar->expressionId, namedVar->type, namedVar->name->text );

    if( definition.initValue ) {
        Expression initValue( *definition.initValue );

        initValue.buildAst( ctx, ExpectedType( namedVar->type ) );
        initValue.codeGen( codeGen );

        codeGen->assign( namedVar->expressionId, initValue.getId() );
    } else {
        ABORT() << "TODO Type default values not yet implemented";
    }
}

CompoundStatement::CompoundStatement(LookupContext *parentCtx, const NonTerminals::CompoundStatement *nt) :
    ctx(parentCtx), parserStatement(nt)
{
}

void CompoundStatement::symbolsPass1() {
    // TODO implement
}

void CompoundStatement::symbolsPass2() {
    // TODO implement
}

void CompoundStatement::codeGen(FunctionGen *codeGen) {
    ABORT()<<"TODO implement";
    for( const auto &parserStatement: parserStatement->statements.statements ) {
        Statement statement( ctx, parserStatement );
        statement.codeGen(codeGen);
    }
}

FuncDecl::FuncDecl(const NonTerminals::FuncDeclBody *nt, LookupContext::Function *function)
    : parserFuncDecl(nt), ctxFunction(function)
{
}

void FuncDecl::symbolsPass1(LookupContext *ctx) {
    // TODO implement ?
}

void FuncDecl::symbolsPass2(LookupContext *ctx) {
    Type retType(&parserFuncDecl->returnType.type);
    retType.symbolsPass2(ctx);
    ctxFunction->returnType = std::move(retType).removeType();

    ctxFunction->arguments.reserve( parserFuncDecl->arguments.arguments.size() );
    for( const auto &parserArg : parserFuncDecl->arguments.arguments ) {
        Type argumentType(&parserArg.type);
        // Function argument types are looked up in the context of the function's parent
        argumentType.symbolsPass2( ctx->getParent() );
        Expression lvalueExpression( std::move(argumentType).removeType() );
        const LookupContext::LocalVariable *localVariable = ctx->registerVariable(
                LookupContext::LocalVariable(
                    parserArg.name.identifier, std::move(lvalueExpression) ) );
        ctxFunction->arguments.emplace_back(
                localVariable->type, localVariable->name->text, localVariable->expressionId );
    }
}

FuncDef::FuncDef(const NonTerminals::FuncDef *nt, LookupContext *ctx, LookupContext::Function *ctxFunction)
    : parserFuncDef(nt), declaration(&nt->decl, ctxFunction)
{
    const NonTerminals::CompoundExpression *expr = std::get_if<NonTerminals::CompoundExpression>( &nt->body );
    if( expr ) {
        body.emplace<CompoundExpression>(ctx, expr);
    } else {
        const NonTerminals::CompoundStatement *stmt = std::get_if<NonTerminals::CompoundStatement>( &nt->body );
        ASSERT( stmt!=nullptr )<<" Function definition is neither statement nor expression";
        body.emplace<CompoundStatement>(ctx, stmt);
    }
}

String FuncDef::getName() const {
    return parserFuncDef->getName();
}

IdentifierId FuncDef::getId() const {
    return id;
}

void FuncDef::setId(IdentifierId id) {
    this->id = id;
}

void FuncDef::symbolsPass1(LookupContext *ctx) {
    declaration.symbolsPass1(ctx);

    struct Visitor {
        void operator()( const std::monostate state ) {
            ABORT()<<"symbolsPass1 on FuncDef with no valid body";
        }

        void operator()( CompoundExpression &expression ) {
            expression.symbolsPass1();
        }

        void operator()( CompoundStatement &statement ) {
            statement.symbolsPass1();
        }
    };

    std::visit( Visitor{}, body );
}

void FuncDef::symbolsPass2(LookupContext *ctx) {
    declaration.symbolsPass2(ctx);

    struct Visitor {
        void operator()( const std::monostate state ) {
            ABORT()<<"symbolsPass1 on FuncDef with no valid body";
        }

        void operator()( CompoundExpression &expression ) {
            expression.symbolsPass2();
        }

        void operator()( CompoundStatement &statement ) {
            statement.symbolsPass2();
        }
    };

    std::visit( Visitor{}, body );
}

void FuncDef::codeGen(PracticalSemanticAnalyzer::ModuleGen *moduleGen) {
    std::shared_ptr<FunctionGen> functionGen = moduleGen->handleFunction(id);

    if( functionGen==nullptr )
        return;

    functionGen->functionEnter(
            id,
            declaration.getName(),
            declaration.getRetType(),
            declaration.getArguments(),
            toSlice("No file"), declaration.getLine(), declaration.getCol());

    struct Visitor {
        FunctionGen *functionGen;
        FuncDecl &declaration;

        void operator()( const std::monostate state ) {
            ABORT()<<"symbolsPass1 on FuncDef with no valid body";
        }

        void operator()( CompoundExpression &expression ) {
            expression.buildAst( ExpectedType( declaration.getRetType() ) );
            Expression result = expression.codeGen( functionGen );
            functionGen->returnValue(result.getId());
        }

        void operator()( CompoundStatement &statement ) {
            statement.codeGen( functionGen );
            functionGen->returnValue();
        }
    };

    std::visit( Visitor{ .functionGen = functionGen.get(), .declaration = declaration }, body );

    functionGen->functionLeave(id);
}

Module::Module( NonTerminals::Module *parseModule, LookupContext *parentSymbolsTable )
    : parseModule(parseModule), ctx(parentSymbolsTable), id(moduleIdAllocator.allocate())
{}

void Module::symbolsPass1() {
    for( auto &symbol : parseModule->functionDefinitions ) {
        FuncDef *funcDef = nullptr;

        auto previousDefinition = ctx.lookupIdentifier(symbol.getName());
        if( previousDefinition == nullptr ) {
            funcDef = &functionDefinitions.emplace_back(
                    &symbol, &ctx, ctx.registerFunctionPass1( symbol.decl.name.identifier ));
        } else {
            ABORT() << "TODO Overloads not yet implemented";
        }

        funcDef->symbolsPass1(&ctx);
    }
}

void Module::symbolsPass2() {
    for( auto &funcDef: functionDefinitions ) {
        funcDef.symbolsPass2(&ctx);
    }
}

void Module::codeGen(PracticalSemanticAnalyzer::ModuleGen *codeGenCB) {
    codeGenCB->moduleEnter(id, toSlice("OnlyModule"), toSlice("No file"), 1, 1);

    for(auto &function: functionDefinitions) {
        function.codeGen(codeGenCB);
    }

    codeGenCB->moduleLeave(id);
}

} // Namespace AST
