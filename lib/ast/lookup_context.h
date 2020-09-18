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

#include "ast/static_type.h"
#include "parser.h"
#include "slice.h"
#include "tokenizer.h"

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


            const Tokenizer::Token *token;
            StaticTypeImpl::CPtr type;
            std::string mangledName;
            CodeGenProto *codeGen = nullptr;
            VrpProto *calcVrp = nullptr;

            Definition( const std::string &name ) : mangledName(name) {}
            explicit Definition( const Tokenizer::Token *token ) : token(token) {}

            StaticType::CPtr returnType() const;
        };

        std::unordered_map<const Tokenizer::Token *, unsigned> firstPassOverloads;
        std::vector<Definition> overloads;
    };

    using Identifier = std::variant<Variable, Function>;

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

    // Members
    std::unordered_map< std::string, StaticTypeImpl::CPtr > types;
    const LookupContext *parent = nullptr;

    std::unordered_map< String, Identifier > symbols;

    std::unordered_map<
            PracticalSemanticAnalyzer::StaticType::CPtr,
            std::unordered_map< PracticalSemanticAnalyzer::StaticType::CPtr, CastDescriptor >
    > typeConversionsFrom;

    std::unordered_map<
            PracticalSemanticAnalyzer::StaticType::CPtr,
            std::unordered_set< PracticalSemanticAnalyzer::StaticType::CPtr >
    > typeConversionsTo;

public:
    explicit LookupContext(const LookupContext *parent = nullptr) :
        parent(parent)
    {}

    const LookupContext *getParent() const {
        return parent;
    }

    StaticTypeImpl::CPtr lookupType( String name ) const;
    StaticTypeImpl::CPtr lookupType( const NonTerminals::Type &type ) const;

    StaticTypeImpl::CPtr registerScalarType( ScalarTypeImpl &&type, ValueRangeBase::CPtr defaultValueRange );

    void addBuiltinFunction(
            const std::string &name, StaticTypeImpl::CPtr returnType, Slice<const StaticTypeImpl::CPtr> argumentTypes,
            Function::Definition::CodeGenProto *codeGen,
            Function::Definition::VrpProto *calcVrp

        );

    void addFunctionPass1( const Tokenizer::Token *token );
    void addFunctionPass2( const Tokenizer::Token *token, StaticTypeImpl::CPtr type );

    void addLocalVar( const Tokenizer::Token *token, StaticTypeImpl::CPtr type, ExpressionId lvalue );

    const Identifier *lookupIdentifier( String name ) const;

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
};

} // End namespace AST

#endif // AST_LOOKUP_CONTEXT_H
