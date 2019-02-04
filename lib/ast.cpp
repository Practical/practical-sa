#include "ast.h"

#include <sstream>

PracticalSemanticAnalyzer::ExpressionId::Allocator<> expressionIdAllocator;
PracticalSemanticAnalyzer::ExpressionId voidExpressionId;
PracticalSemanticAnalyzer::ModuleId::Allocator<> moduleIdAllocator;

namespace AST {

LookupContext AST::globalCtx(nullptr);

ImplicitCastNotAllowed::ImplicitCastNotAllowed(const StaticType *src, const StaticType *dst, size_t line, size_t col)
        : compile_error(line, col)
{
    std::stringstream buf;

    buf<<"Cannot implicitly cast from "<<*src<<" to "<<*dst;

    setMsg( buf.str().c_str() );
}

StaticType AST::deductLiteralRange(LongEnoughInt value) {
    String typeName;

    if( value<0 ) {
        if( value>=INT8_MIN ) {
            typeName = toSlice("S8");
        } else if( value>=INT16_MIN ) {
            typeName = toSlice("S16");
        } else if( value>=INT32_MIN ) {
            typeName = toSlice("S32");
        } else if( value>=INT64_MIN ) {
            typeName = toSlice("S64");
        } else {
            abort(); // XXX implement
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
            abort(); // XXX implement
        }
    }

    auto object = globalCtx.getSymbol(typeName);
    assert(object != nullptr); // Couldn't look up built-in types
    const BuiltInType *type = std::get_if<BuiltInType>(&object->definition);
    assert(type != nullptr); // The built-in type is defined by name, but not of type built-in type

    return StaticType( type->id );
}

void AST::prepare()
{
    // Register the built-in types
    globalCtx.registerBuiltInType( BuiltInType("Void", BuiltInType::Type::Void, 0) );
    voidExpressionId = expressionIdAllocator.allocate();

    globalCtx.registerBuiltInType( BuiltInType("S8", BuiltInType::Type::SignedInt, 8) );
    globalCtx.registerBuiltInType( BuiltInType("S16", BuiltInType::Type::SignedInt, 16) );
    globalCtx.registerBuiltInType( BuiltInType("S32", BuiltInType::Type::SignedInt, 32) );
    globalCtx.registerBuiltInType( BuiltInType("S64", BuiltInType::Type::SignedInt, 64) );
    globalCtx.registerBuiltInType( BuiltInType("S128", BuiltInType::Type::SignedInt, 128) );

    globalCtx.registerBuiltInType( BuiltInType("Bool", BuiltInType::Type::Boolean, 1) );

    globalCtx.registerBuiltInType( BuiltInType("U8", BuiltInType::Type::UnsignedInt, 8) );
    globalCtx.registerBuiltInType( BuiltInType("U16", BuiltInType::Type::UnsignedInt, 16) );
    globalCtx.registerBuiltInType( BuiltInType("U32", BuiltInType::Type::UnsignedInt, 32) );
    globalCtx.registerBuiltInType( BuiltInType("U64", BuiltInType::Type::UnsignedInt, 64) );
    globalCtx.registerBuiltInType( BuiltInType("U128", BuiltInType::Type::UnsignedInt, 128) );

    // Types for internal use only
    globalCtx.registerBuiltInType( BuiltInType("__U7", BuiltInType::Type::InternalUnsignedInt, 7) );
    globalCtx.registerBuiltInType( BuiltInType("__U15", BuiltInType::Type::InternalUnsignedInt, 15) );
    globalCtx.registerBuiltInType( BuiltInType("__U31", BuiltInType::Type::InternalUnsignedInt, 31) );
    globalCtx.registerBuiltInType( BuiltInType("__U63", BuiltInType::Type::InternalUnsignedInt, 63) );
    globalCtx.registerBuiltInType( BuiltInType("__U127", BuiltInType::Type::InternalUnsignedInt, 127) );
}

void AST::parseModule(String moduleSource) {
    auto module = safenew<NonTerminals::Module>();
    module->parse(moduleSource);

    auto parseModule = parsedModules.emplace(module->getName(), std::move(module));
    assert( parseModule.second ); // module already existed

    auto astModule = modulesAST.emplace(
            parseModule.first->second->getName(),
            new ::AST::Module( parseModule.first->second.get(), &globalCtx )
            );
    assert( astModule.second ); // Module already existed

    astModule.first->second->symbolsPass1();
    astModule.first->second->symbolsPass2();
}

void AST::codeGen(PracticalSemanticAnalyzer::ModuleGen *codeGen) {
    // XXX We should only code-gen some of the modules?
    for(auto &module: modulesAST) {
        module.second->codeGen(codeGen);
    }
}

bool implicitCastAllowed(const StaticType &sourceType, const StaticType &destType, const LookupContext &ctx) {
    // Fastpath
    if( sourceType==destType )
        return true;

    const LookupContext::NamedObject *namedSource = ctx.getSymbol( sourceType.getId() );
    const LookupContext::NamedObject *namedDest = ctx.getSymbol( destType.getId() );

    assert( namedSource->definition.index()==LookupContext::NamedObject::Type::BuiltInType );
    assert( namedDest->definition.index()==LookupContext::NamedObject::Type::BuiltInType );

    const BuiltInType *builtinSource = &std::get<BuiltInType>( namedSource->definition );
    const BuiltInType *builtinDest = &std::get<BuiltInType>( namedDest->definition );

    if( builtinSource->type==BuiltInType::Type::SignedInt ) {
        if( builtinDest->type!=BuiltInType::Type::SignedInt )
            return false;

        return builtinSource->sizeInBits <= builtinDest->sizeInBits;
    }

    if( builtinSource->type==BuiltInType::Type::UnsignedInt || builtinSource->type==BuiltInType::Type::InternalUnsignedInt ) {
        if( builtinDest->type==BuiltInType::Type::SignedInt )
            return builtinSource->sizeInBits < builtinDest->sizeInBits;

        if( builtinDest->type==BuiltInType::Type::UnsignedInt || builtinDest->type==BuiltInType::Type::InternalUnsignedInt ) {
            return builtinSource->sizeInBits <= builtinDest->sizeInBits;
        }
    }

    abort(); // Shouldn't reach here
}

} // Namespace AST
