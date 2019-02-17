#include "ast_nodes.h"

#include "ast.h"
#include "practical-errors.h"

#include "asserts.h"

using PracticalSemanticAnalyzer::CannotTakeValueOfFunction;

namespace AST {

Type::Type(const NonTerminals::Type *nt) : parseType(nt) {
}

void Type::symbolsPass2(LookupContext *ctx) {
    auto type = ctx->lookupType(parseType->type.text);
    if( type==nullptr )
        throw pass2_error("Type name expected", parseType->getLine(), parseType->getCol());

    staticType.setId( type->id() );
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

    struct Visitor {
        CompoundExpression *_this;
        FunctionGen *codeGen;

        void operator()(const NonTerminals::Expression &expression) {
            _this->codeGenExpression(codeGen, nullptr, &expression); // Disregard return value
        }

        void operator()(const NonTerminals::VariableDefinition &definition) {
            _this->codeGenVarDef(codeGen, &definition);
        }

        void operator()(std::monostate mono) {
            ABORT() << "Codegen called on unparsed expression";
        }
    } visitor = { this, codeGen };

    std::visit( visitor, statement->content );
}

ExpressionId CompoundExpression::codeGenExpression(
        FunctionGen *codeGen, const StaticType *expectedResult, const NonTerminals::Expression *expression)
{
    using namespace NonTerminals;

    struct Visitor {
        CompoundExpression *_this;
        FunctionGen *codeGen;
        const StaticType *expectedResult;

        ExpressionId operator()( std::monostate none ) const {
            return voidExpressionId;
        }

        ExpressionId operator()( const std::unique_ptr<NonTerminals::CompoundExpression> &compound ) const {
            ABORT() << "TODO implement";
            /*
            return std::get< static_cast<unsigned int>(Expression::ExpressionType::CompoundExpression) >(expression->value)->
                    codeGen(functionGen);
            */
        }

        ExpressionId operator()( const NonTerminals::Literal &literal ) const {
            return _this->codeGenLiteral( codeGen, expectedResult, &literal );
        }

        ExpressionId operator()( const Tokenizer::Token *identifier ) {
            return _this->codeGenIdentifierLookup(codeGen, expectedResult, identifier);
        }
    };

    Visitor visitor = { this, codeGen, expectedResult };
    return std::visit( visitor, expression->value );

    // return voidExpressionId;
}

void CompoundExpression::codeGenVarDef(FunctionGen *codeGen, const NonTerminals::VariableDefinition *definition) {
    Type varType( &definition->body.type );
    varType.symbolsPass2( &ctx );
    const LookupContext::LocalVariable *namedVar = ctx.registerVariable( LookupContext::LocalVariable(
			    definition->body.name, std::move(varType).removeType(), expressionIdAllocator.allocate() ) );
    codeGen->allocateStackVar( namedVar->lvalueId, namedVar->type, namedVar->name->text );

    if( definition->initValue ) {
        ExpressionId initValueId = codeGenExpression(codeGen, &namedVar->type, definition->initValue.get());
        codeGen->assign( namedVar->lvalueId, initValueId );
    } else {
        ABORT() << "TODO Type default values not yet implemented";
    }
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

            ASSERT( c>='0' && c<='9' ) << "Decimal literal has character '"<<c<<"' out of allowed range";
            res *= 10;
            res += c-'0';
        }
        break;
    default:
        ABORT() << "TODO implement";
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

ExpressionId CompoundExpression::codeGenIdentifierLookup(
        FunctionGen *codeGen, const StaticType *expectedResult, const Tokenizer::Token *identifier)
{
    const LookupContext::NamedObject *referencedObject = ctx.lookupIdentifier( identifier->text );
    if( referencedObject==nullptr )
        throw SymbolNotFound(identifier->text, identifier->line, identifier->col);

    struct Visitor {
        const Tokenizer::Token *identifier;
        FunctionGen *codeGen;
        const StaticType *expectedResult;

        ExpressionId operator()(const LookupContext::LocalVariable &localVar) const {
            ExpressionId expId = expressionIdAllocator.allocate();
            codeGen->dereferencePointer( expId, localVar.type, localVar.lvalueId );

            /* TODO implement
            if( expectedResult!=nullptr )
                expId = codeGenImplicitCast( expId, localVar.type, *expectedResult, identifier );
            */

            return expId;
        }

        ExpressionId operator()(const LookupContext::Function &function) const {
            throw CannotTakeValueOfFunction(identifier);
        }
    };

    return std::visit(
            Visitor{ .identifier=identifier, .codeGen=codeGen, .expectedResult=expectedResult }, *referencedObject );
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
            Slice<ArgumentDeclaration>(),
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

        auto previousDefinition = ctx.lookupIdentifier(symbol.getName());
        if( previousDefinition == nullptr ) {
            ctx.registerFunctionPass1( &symbol.decl.name );
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
