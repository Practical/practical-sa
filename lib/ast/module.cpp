/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "module.h"

#include "ast/function.h"

#include <practical/errors.h>

namespace AST {

static ModuleId::Allocator<> moduleIdAllocator;

Module::Module( const NonTerminals::Module &parserModule, const LookupContext &parentLookupContext ) :
    parserModule(parserModule),
    lookupContext(&parentLookupContext),
    moduleId( moduleIdAllocator.allocate() )
{} 

void Module::symbolsPass1() {
    for( const auto &structDef : parserModule.structureDefinitions ) {
        lookupContext.addStructPass1( structDef );
    }

    for( const auto &funcDecl : parserModule.functionDeclarations ) {
        lookupContext.addFunctionDeclarationPass1( funcDecl.decl.name.identifier );
    }

    for( const auto &funcDef : parserModule.functionDefinitions ) {
        lookupContext.addFunctionDefinitionPass1( funcDef.decl.name.identifier );
    }
}

void Module::symbolsPass2() {
    AST::DelayedDefinitions delayedDefs;

    for( const auto &structDef : parserModule.structureDefinitions ) {
        lookupContext.addStructPass2( structDef, delayedDefs );
    }

    while( ! delayedDefs.ready.empty() ) {
        auto i = delayedDefs.ready.begin();
        std::pair< const NonTerminals::StructDef *, LookupContext * > ready = *i;
        delayedDefs.ready.erase(i);
        ready.second->addStructPass2( *ready.first, delayedDefs );
    }

    if( !delayedDefs.pending.empty() ) {
        throw CircularDependency( delayedDefs.pending.begin()->first->keyword->location );
    }

    for( const auto &funcDecl : parserModule.functionDeclarations ) {
        StaticTypeImpl::CPtr funcType = constructFunctionType( funcDecl.decl );

        if( funcDecl.abiSpecifier.token != nullptr ) {
            lookupContext.addFunctionDeclarationPass2(
                    funcDecl.decl.name.identifier,
                    funcType,
                    LookupContext::parseAbiString(
                        funcDecl.abiSpecifier.value, funcDecl.abiSpecifier.token->location)
                );
        } else {
            lookupContext.addFunctionDeclarationPass2( funcDecl.decl.name.identifier, funcType );
        }
    }

    for( const auto &funcDef : parserModule.functionDefinitions ) {
        lookupContext.addFunctionDefinitionPass2(
                funcDef.decl.name.identifier, constructFunctionType(funcDef.decl) );
    }
}

void Module::codeGen( PracticalSemanticAnalyzer::ModuleGen *moduleGen ) {
    moduleGen->moduleEnter( moduleId, "Module", "file.pr", 1, 1 );

    lookupContext.declareStructs( moduleGen );
    lookupContext.declareFunctions( moduleGen );

    lookupContext.defineStructs( moduleGen );

    std::vector<Function> functions;
    functions.reserve( parserModule.functionDefinitions.size() );

    for( const auto &funcDef : parserModule.functionDefinitions ) {
        functions.emplace_back( funcDef, lookupContext );
    }

    for( auto &function : functions ) {
        function.codeGen( moduleGen->handleFunction() );
    }

    moduleGen->moduleLeave( moduleId );
}

StaticTypeImpl::CPtr Module::constructFunctionType( const NonTerminals::FuncDeclBody &decl ) const {
    StaticTypeImpl::CPtr returnType = lookupContext.lookupType( decl.returnType.type );

    std::vector<StaticTypeImpl::CPtr> arguments;
    arguments.reserve( decl.arguments.arguments.size() );
    for( const auto &argument : decl.arguments.arguments ) {
        arguments.emplace_back( lookupContext.lookupType( argument.type ) );
    }

    return StaticTypeImpl::allocate(
            FunctionTypeImpl(
                std::move(returnType),
                std::move(arguments)
            )
        );
}

} // namespace AST
