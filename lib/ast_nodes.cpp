#include "ast_nodes.h"

#include "ast.h"

namespace AST {

Type::Type(const NonTerminals::Type *nt) : parseType(nt) {
}

void Type::symbolsPass2(LookupContext *ctx) {
    auto symbol = ctx->getSymbol(parseType->type.text);
    if( symbol==nullptr || !symbol->isType() )
        throw pass2_error("Type name expected", parseType->getLine(), parseType->getCol());

    staticType.setId( symbol->getId() );
}

CompoundExpression::CompoundExpression(LookupContext *parentCtx, const NonTerminals::CompoundExpression *_parserExpresison)
    : ctx(parentCtx), parserExpression(_parserExpresison)
{
}

void CompoundExpression::symbolsPass1() {
    // TODO implement
}

void CompoundExpression::symbolsPass2() {
    // TODO implement
}

ExpressionId CompoundExpression::codeGen(FunctionGen *codeGen, const StaticType *expectedResult) {
    for( auto &statement: parserExpression->statementList.statements ) {
        codeGenStatement( codeGen, &statement );
    }

    return codeGenExpression( codeGen, expectedResult, &parserExpression->expression );
}

void CompoundExpression::codeGenStatement(FunctionGen *codeGen, const NonTerminals::Statement *statement) {
    codeGenExpression(codeGen, nullptr, &statement->expression); // Disregard return value
}

ExpressionId CompoundExpression::codeGenExpression(
        FunctionGen *codeGen, const StaticType *expectedResult, const NonTerminals::Expression *expression)
{
    using namespace NonTerminals;

    switch( Expression::ExpressionType(expression->value.index()) ) {
    case Expression::ExpressionType::None:
        return voidExpressionId;
    case Expression::ExpressionType::CompoundExpression:
        abort();
        /*
        return std::get< static_cast<unsigned int>(Expression::ExpressionType::CompoundExpression) >(expression->value)->
                codeGen(functionGen);
        */
    case Expression::ExpressionType::Literal:
        return codeGenLiteral( codeGen, expectedResult, &std::get< Expression::ExpressionType::Literal >(expression->value) );
    case Expression::ExpressionType::Identifier:
        abort(); // TODO implement
    }

    return voidExpressionId;
}

ExpressionId CompoundExpression::codeGenLiteral(
        FunctionGen *codeGen, const StaticType *expectedResult, const NonTerminals::Literal *literal)
{
    String text = literal->token.text;

    ExpressionId id = expressionIdAllocator.allocate();

    LongEnoughInt res = 0;

    switch( literal->token.token ) {
    case Tokenizer::Tokens::LITERAL_INT_10:
        for( char c: text ) {
            // TODO check range and assign type

            assert( c>='0' && c<='9' );
            res *= 10;
            res += c-'0';
        }
        break;
    default:
        abort(); // TODO implement
    }

    // XXX use value range propagation instead
    StaticType resType( AST::AST::deductLiteralRange(res) );
    bool useExpected = false;
    if( expectedResult!=nullptr ) {
        if( !implicitCastAllowed(resType, *expectedResult, AST::getGlobalCtx()) )
            throw ImplicitCastNotAllowed(&resType, expectedResult, literal->token.line, literal->token.col);

        useExpected = true;
    }

    codeGen->setLiteral(id, res, useExpected ? *expectedResult : resType);

    return id;
}

FuncDecl::FuncDecl(const NonTerminals::FuncDeclBody *nt)
    : parserFuncDecl(nt), retType(&nt->returnType.type)
{
}

void FuncDecl::symbolsPass1(LookupContext *ctx) {
    // TODO implement ?
}

void FuncDecl::symbolsPass2(LookupContext *ctx) {
    retType.symbolsPass2(ctx);

    // TODO parse the arguments
}

FuncDef::FuncDef(const NonTerminals::FuncDef *nt, LookupContext *ctx)
    : parserFuncDef(nt), declaration(&nt->decl), body(ctx, &nt->body)
{
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
    body.symbolsPass1();
}

void FuncDef::symbolsPass2(LookupContext *ctx) {
    declaration.symbolsPass2(ctx);
    body.symbolsPass2();
}

void FuncDef::codeGen(PracticalSemanticAnalyzer::ModuleGen *moduleGen) {
    std::shared_ptr<FunctionGen> functionGen = moduleGen->handleFunction(id);

    if( functionGen==nullptr )
        return;

    functionGen->functionEnter(
            id,
            declaration.getName(),
            declaration.getRetType(),
            Slice<VariableDeclaration>(),
            toSlice("No file"), declaration.getLine(), declaration.getCol());

    ExpressionId result = body.codeGen( functionGen.get(), &declaration.getRetType() );
    functionGen->returnValue(result);

    functionGen->functionLeave(id);
}

Module::Module( NonTerminals::Module *parseModule, LookupContext *parentSymbolsTable )
    : parseModule(parseModule), ctx(parentSymbolsTable), id(moduleIdAllocator.allocate())
{}

void Module::symbolsPass1() {
    for( auto &symbol : parseModule->functionDefinitions ) {
        FuncDef *funcDef = &functionDefinitions.emplace_back(&symbol, &ctx);
        LookupContext::NamedObject def(funcDef);

        auto previousDefinition = ctx.getSymbol(symbol.getName());
        if( previousDefinition == nullptr ) {
            ctx.addSymbolPass1(symbol.getName(), std::move(def));
        } else {
            // XXX Overloads not yet implemented
            assert(false);
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
