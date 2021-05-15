/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_LOOKUP_CONTEXT_H
#define AST_LOOKUP_CONTEXT_H

#include "ast/delayed_definitions.h"
#include "ast/static_type.h"
#include "ast/struct_member.h"
#include "parser/struct.h"
#include "parser.h"
#include "tokenizer.h"

#include <practical/slice.h>

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace AST {

class Expression;

class LookupContext : private NoCopy {
public:
    struct Variable {
        const Tokenizer::Token *token;
        StaticTypeImpl::CPtr type;
        ExpressionId lvalueId;

        explicit Variable( const Tokenizer::Token *token ) : token(token) {}
        explicit Variable( const Tokenizer::Token *token, StaticTypeImpl::CPtr type, ExpressionId lvalueId ) :
            token(token), type(type), lvalueId(lvalueId)
        {}
    };

    struct Function {
        struct Definition {
            using CodeGenProto =
                    ExpressionId(
                            Slice<const Expression>, const Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
            using VrpProto =
                    ValueRangeBase::CPtr(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);


            const Tokenizer::Token *token = nullptr;
            StaticTypeImpl::CPtr type;
            std::string mangledName;
            CodeGenProto *codeGen = nullptr;
            VrpProto *calcVrp = nullptr;
            bool declarationOnly = true;

            Definition( const Tokenizer::Token *token, const std::string &name ) :
                token(token), mangledName(name)
            {}

            StaticType::CPtr returnType() const;
        };

        using OverloadsContainer = std::unordered_map< StaticTypeImpl::CPtr, Definition >;
        std::unordered_map<const Tokenizer::Token *, OverloadsContainer::const_iterator> firstPassOverloads;
        OverloadsContainer overloads;
    };

    using Identifier = std::variant<Variable, Function, StructMember>;

    using CodeGenCast = ExpressionId (*)(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            PracticalSemanticAnalyzer::FunctionGen *functionGen);
    using ValueRangeCast = ValueRangeBase::CPtr (*)(
            const StaticTypeImpl *sourceType,
            const StaticTypeImpl *destType,
            ValueRangeBase::CPtr inputRange,
            bool isImplicit
        );

    struct CastDescriptor {
        enum class ImplicitCastAllowed {
            Always,
            VrpConditional,
            Never
        };

        StaticTypeImpl::CPtr sourceType, destType;
        CodeGenCast codeGen;
        ValueRangeCast calcVrp = nullptr;
        unsigned weight = std::numeric_limits<unsigned>::max();
        ImplicitCastAllowed whenPossible = ImplicitCastAllowed::Always;

        CastDescriptor(
                StaticTypeImpl::CPtr sourceType,
                StaticTypeImpl::CPtr destType,
                CodeGenCast codeGen, ValueRangeCast calcVrp,
                unsigned weight, ImplicitCastAllowed whenPossible
            ) :
                sourceType(sourceType),
                destType(destType),
                codeGen(codeGen),
                calcVrp(calcVrp),
                weight(weight),
                whenPossible(whenPossible)
        {}
    };

private:
    using CalcValueRangeCast = ValueRangeBase::CPtr (*)(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType,
            ValueRangeBase::CPtr sourceRange,
            PracticalSemanticAnalyzer::StaticType::CPtr destType);

public:
    explicit LookupContext(const LookupContext *parent = nullptr) :
        _parent(parent)
    {}

    const LookupContext *getParent() const {
        return _parent;
    }

private:
    StaticTypeImpl::CPtr lookupType( String name, const SourceLocation &location ) const;
public:
    StaticTypeImpl::CPtr lookupType( String name ) const;
    StaticTypeImpl::CPtr lookupType( const NonTerminals::Type &type ) const;
    StaticTypeImpl::CPtr lookupType( const NonTerminals::TransientType &type ) const;

    StaticTypeImpl::CPtr registerScalarType( ScalarTypeImpl &&type, ValueRangeBase::CPtr defaultValueRange );

    void addBuiltinFunction(
            const std::string &name, StaticTypeImpl::CPtr returnType, Slice<const StaticTypeImpl::CPtr> argumentTypes,
            Function::Definition::CodeGenProto *codeGen,
            Function::Definition::VrpProto *calcVrp

        );

    enum class AbiType { Practical, C };
    friend std::ostream &operator<<( std::ostream &out, AbiType abi );

    void addFunctionDeclarationPass1( const Tokenizer::Token *token );
    void addFunctionDefinitionPass1( const Tokenizer::Token *token );
    void addStructPass1( const NonTerminals::StructDef &token );
    void addFunctionDeclarationPass2(
            const Tokenizer::Token *token, StaticTypeImpl::CPtr type, AbiType abi = AbiType::Practical );
    void addFunctionDefinitionPass2(
            const Tokenizer::Token *token, StaticTypeImpl::CPtr type, AbiType abi = AbiType::Practical );
    void addStructPass2( const NonTerminals::StructDef &token, DelayedDefinitions &delayedDefs );

    void declareFunctions( PracticalSemanticAnalyzer::ModuleGen *moduleGen ) const;
    void declareStructs( PracticalSemanticAnalyzer::ModuleGen *moduleGen ) const;
    void defineStructs( PracticalSemanticAnalyzer::ModuleGen *moduleGen ) const;

    static AbiType parseAbiString( String abiString, const SourceLocation &location );

    void addLocalVar( const Tokenizer::Token *token, StaticTypeImpl::CPtr type, ExpressionId lvalue );
    String addStructMember(
            const Tokenizer::Token *token, StaticTypeImpl::CPtr type, size_t offset );

    const Identifier *lookupIdentifier( String name ) const;

    // Generic type and range to use for unspecified function
    static StaticTypeImpl::CPtr genericFunctionType();
    static ValueRangeBase::CPtr genericFunctionRange();

    void addCast(
            StaticTypeImpl::CPtr sourceType,
            StaticTypeImpl::CPtr destType,
            unsigned weight, CodeGenCast codeGenCast, ValueRangeCast calcVrp,
            CastDescriptor::ImplicitCastAllowed whenPossible );

    class CastsList : private NoCopy {
        friend LookupContext;

        std::vector<const CastDescriptor *> casts;
        unsigned index = 0;

    public:

        explicit operator bool() const {
            return index<casts.size();
        }

        void operator++() {
            ++index;
        }

        const CastDescriptor &operator*() const {
            return *casts.at(index);
        }

        const CastDescriptor *operator->() const {
            return casts.at(index);
        }

        const CastDescriptor *get() const {
            return casts.at(index);
        }

        size_t size() const {
            return casts.size();
        }

    private:
        void sort();
    };

    const CastDescriptor *lookupCast(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType,
            PracticalSemanticAnalyzer::StaticType::CPtr destType ) const;

    CastsList allCastsTo( PracticalSemanticAnalyzer::StaticType::CPtr destType ) const;
    CastsList allCastsFrom( PracticalSemanticAnalyzer::StaticType::CPtr sourceType ) const;

private:
    static ExpressionId globalFunctionCall(
            Slice<const Expression>,
            const Function::Definition *definition,
            PracticalSemanticAnalyzer::FunctionGen *functionGen);

    Function::Definition &addFunctionPass2(
            const Tokenizer::Token *token, StaticTypeImpl::CPtr type, AbiType abi, bool isDefinition );

    // Members
    static StaticTypeImpl::CPtr _genericFunctionType;
    static ValueRangeBase::CPtr _genericFunctionRange;

    std::unordered_map< std::string, StaticTypeImpl::Ptr > _typesUnderConstruction;
    std::unordered_map< std::string, StaticTypeImpl::CPtr > _types;
    const LookupContext *_parent = nullptr;

    std::unordered_map< String, Identifier > _symbols;

    std::unordered_map<
            PracticalSemanticAnalyzer::StaticType::CPtr,
            std::unordered_map< PracticalSemanticAnalyzer::StaticType::CPtr, CastDescriptor >
    > _typeConversionsFrom;

    std::unordered_map<
            PracticalSemanticAnalyzer::StaticType::CPtr,
            std::unordered_set< PracticalSemanticAnalyzer::StaticType::CPtr >
    > _typeConversionsTo;
};

} // End namespace AST

#endif // AST_LOOKUP_CONTEXT_H
