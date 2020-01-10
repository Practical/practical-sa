/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast.h"

#include "asserts.h"

#include <string>
#include <sstream>

Expression voidExpression;
StaticType::Ptr typeType;
ModuleId::Allocator<> moduleIdAllocator;

namespace AST {

LookupContext AST::builtinCtx(nullptr);

StaticType::Ptr AST::deductLiteralRange(LongEnoughInt value) {
    String typeName;

    if( value<0 ) {
        if( value>=(LongEnoughInt)INT8_MIN ) {
            typeName = toSlice("S8");
        } else if( value>=(LongEnoughInt)INT16_MIN ) {
            typeName = toSlice("S16");
        } else if( value>=(LongEnoughInt)INT32_MIN ) {
            typeName = toSlice("S32");
        } else if( value>=(LongEnoughInt)INT64_MIN ) {
            typeName = toSlice("S64");
        } else {
            ABORT() << "TODO implement";
        }
    } else {
        if( value<=UINT8_MAX ) {
            typeName = toSlice("U8");
        } else if( value<=UINT16_MAX ) {
            typeName = toSlice("U16");
        } else if( value<=UINT32_MAX ) {
            typeName = toSlice("U32");
        } else if( value<=UINT64_MAX ) {
            typeName = toSlice("U64");
        } else {
            ABORT() << "XXX implement";
        }
    }

    auto type = builtinCtx.lookupType(typeName);
    ASSERT(type != nullptr) << "Failed to look up built-in type \"" << typeName << "\"";

    return StaticType::allocate( type->id() );
}

class BuiltInTypeToken : public Tokenizer::Token {
public:
    BuiltInTypeToken(String name) {
        text = name;
        token = Tokenizer::Tokens::IDENTIFIER;
    }
};

void AST::prepare()
{
    // Register the built-in types
#define RegisterBuiltInType( name, type, size ) \
    static const BuiltInTypeToken name##Identifier{String(#name, std::char_traits<char>::length(#name))}; \
    builtinCtx.registerType( &name##Identifier, NamedType::Type::type, size )
#define RegisterBuiltInTypeWithRange( name, type, size, minvalue, maxvalue ) \
    static const BuiltInTypeToken name##Identifier{String(#name, std::char_traits<char>::length(#name))}; \
    builtinCtx.registerType( &name##Identifier, NamedType::Type::type, size, minvalue, maxvalue )

    RegisterBuiltInType( Void, Void, 0 );
    TypeId VoidTypeId = builtinCtx.lookupType( "Void" )->id();
    voidExpression = Expression( StaticType::allocate( VoidTypeId ) );

    RegisterBuiltInType( Type, Type, 0 );
    TypeId TypeTypeId = builtinCtx.lookupType( "Type" )->id();
    typeType = StaticType::allocate( TypeTypeId );

    RegisterBuiltInType( Bool, Boolean, 1 );

    RegisterBuiltInTypeWithRange( S8, SignedInteger, 8, INT8_MIN, INT8_MAX );
    RegisterBuiltInTypeWithRange( S16, SignedInteger, 16, INT16_MIN, INT16_MAX );
    RegisterBuiltInTypeWithRange( S32, SignedInteger, 32, INT32_MIN, INT32_MAX );
    RegisterBuiltInTypeWithRange( S64, SignedInteger, 64, INT64_MIN, INT64_MAX );
    //RegisterBuiltInTypeWithRange( S128, SignedInteger, 128 );

    RegisterBuiltInTypeWithRange( U8, UnsignedInteger, 8, 0, UINT8_MAX );
    RegisterBuiltInTypeWithRange( U16, UnsignedInteger, 16, 0, UINT16_MAX );
    RegisterBuiltInTypeWithRange( U32, UnsignedInteger, 32, 0, UINT32_MAX );
    RegisterBuiltInTypeWithRange( U64, UnsignedInteger, 64, 0, UINT64_MAX );
    //RegisterBuiltInTypeWithRange( U128, UnsignedInteger, 128 );

#undef RegisterBuiltInType
#undef RegisterBuiltInTypeWithRange
}

void AST::parseModule(String moduleSource) {
    auto module = safenew<NonTerminals::Module>();
    module->parse(moduleSource);

    auto parseModule = parsedModules.emplace(module->getName(), std::move(module));
    ASSERT( parseModule.second ) << "module \"" << module->getName() << "\" already existed";

    auto astModule = modulesAST.emplace(
            parseModule.first->second->getName(),
            new ::AST::Module( parseModule.first->second.get(), &builtinCtx )
            );
    ASSERT( astModule.second ) << "Module \"" << parseModule.first->second->getName() << "\"already existed";

    astModule.first->second->symbolsPass1();
    astModule.first->second->symbolsPass2();
}

void AST::codeGen(ModuleGen *codeGen) {
    // XXX We should only code-gen some of the modules?
    for(auto &module: modulesAST) {
        module.second->codeGen(codeGen);
    }
}

} // Namespace AST
