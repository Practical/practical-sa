#include "ast.h"

#include "asserts.h"

#include <string>
#include <sstream>

ExpressionId::Allocator<> expressionIdAllocator;
ExpressionId voidExpressionId;
ModuleId::Allocator<> moduleIdAllocator;

namespace AST {

LookupContext AST::globalCtx(nullptr);

StaticType AST::deductLiteralRange(LongEnoughInt value) {
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
        if( value<=INT8_MAX ) {
            typeName = toSlice("__U7");
        } else if( value<=UINT8_MAX ) {
            typeName = toSlice("U8");
        } else if( value<=INT16_MAX ) {
            typeName = toSlice("__U15");
        } else if( value<=UINT16_MAX ) {
            typeName = toSlice("U16");
        } else if( value<=INT32_MAX ) {
            typeName = toSlice("__U31");
        } else if( value<=UINT32_MAX ) {
            typeName = toSlice("U32");
        } else if( value<=INT64_MAX ) {
            typeName = toSlice("__U63");
        } else if( value<=UINT64_MAX ) {
            typeName = toSlice("U64");
        } else {
            ABORT() << "XXX implement";
        }
    }

    auto type = globalCtx.lookupType(typeName);
    ASSERT(type != nullptr) << "Failed to look up built-in type \"" << typeName << "\"";

    return StaticType( type->id() );
}

class BuiltInTypeToken : public Tokenizer::Token {
public:
    constexpr BuiltInTypeToken(String name) {
        text = name;
        token = Tokenizer::Tokens::IDENTIFIER;
    }
};

void AST::prepare()
{
    // Register the built-in types
#define RegisterBuiltInType( name, type, size ) \
    static constexpr BuiltInTypeToken name##Identifier{String(#name, std::char_traits<char>::length(#name))}; \
    globalCtx.registerType( &name##Identifier, NamedType::Type::type, size )

    RegisterBuiltInType( Void, Void, 0 );
    voidExpressionId = expressionIdAllocator.allocate();

    RegisterBuiltInType( Bool, Boolean, 1 );

    RegisterBuiltInType( S8, SignedInteger, 8 );
    RegisterBuiltInType( S16, SignedInteger, 16 );
    RegisterBuiltInType( S32, SignedInteger, 32 );
    RegisterBuiltInType( S64, SignedInteger, 64 );
    RegisterBuiltInType( S128, SignedInteger, 128 );

    RegisterBuiltInType( U8, UnsignedInteger, 8 );
    RegisterBuiltInType( U16, UnsignedInteger, 16 );
    RegisterBuiltInType( U32, UnsignedInteger, 32 );
    RegisterBuiltInType( U64, UnsignedInteger, 64 );
    RegisterBuiltInType( U128, UnsignedInteger, 128 );

    // Types for internal use only
    RegisterBuiltInType( __U7, UnsignedInteger, 7 );
    RegisterBuiltInType( __U15, UnsignedInteger, 15 );
    RegisterBuiltInType( __U31, UnsignedInteger, 31 );
    RegisterBuiltInType( __U63, UnsignedInteger, 63 );
    RegisterBuiltInType( __U127, UnsignedInteger, 127 );

#undef RegisterBuiltInType
}

void AST::parseModule(String moduleSource) {
    auto module = safenew<NonTerminals::Module>();
    module->parse(moduleSource);

    auto parseModule = parsedModules.emplace(module->getName(), std::move(module));
    ASSERT( parseModule.second ) << "module \"" << module->getName() << "\" already existed";

    auto astModule = modulesAST.emplace(
            parseModule.first->second->getName(),
            new ::AST::Module( parseModule.first->second.get(), &globalCtx )
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
