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

namespace AST {

static ModuleId::Allocator<> moduleIdAllocator;

Module::Module( const NonTerminals::Module &parserModule, const LookupContext &parentLookupContext ) :
    parserModule(parserModule),
    lookupContext(&parentLookupContext),
    moduleId( moduleIdAllocator.allocate() )
{} 

void Module::symbolsPass1() {
    for( const auto &funcDef : parserModule.functionDefinitions ) {
        lookupContext.addFunctionPass1( funcDef.decl.name.identifier );
    }
}

void Module::symbolsPass2() {
    for( const auto &funcDef : parserModule.functionDefinitions ) {
        const NonTerminals::FuncDeclBody &decl = funcDef.decl;

        StaticTypeImpl::CPtr returnType = lookupContext.lookupType( decl.returnType.type );

        std::vector<StaticTypeImpl::CPtr> arguments;
        arguments.reserve( decl.arguments.arguments.size() );
        for( const auto &argument : decl.arguments.arguments ) {
            arguments.emplace_back( lookupContext.lookupType( argument.type ) );
        }

        lookupContext.addFunctionPass2(
                decl.name.identifier,
                StaticTypeImpl::allocate(
                    FunctionTypeImpl(
                        std::move(returnType),
                        std::move(arguments),
                        decl.name.identifier->text
                    )
                ) );
    }
}

void Module::codeGen( PracticalSemanticAnalyzer::ModuleGen *moduleGen ) {
    moduleGen->moduleEnter( moduleId, "Module", "file.pr", 1, 1 );

    for( const auto &funcDef : parserModule.functionDefinitions ) {
        Function function(funcDef, lookupContext);

        function.codeGen( moduleGen->handleFunction() );
    }

    moduleGen->moduleLeave( moduleId );
}

} // namespace AST
