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

class LookupContext : private NoCopy {
public:
    struct Symbol {
        const Tokenizer::Token *token;
        StaticTypeImpl::CPtr type;
        ExpressionId lvalueId;

        explicit Symbol( const Tokenizer::Token *token ) : token(token) {}
        explicit Symbol( const Tokenizer::Token *token, StaticTypeImpl::CPtr type, ExpressionId lvalueId ) :
            token(token), type(type), lvalueId(lvalueId)
        {}
    };

    using CodeGenCast = ExpressionId (*)(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            PracticalSemanticAnalyzer::FunctionGen *functionGen);

private:
    using CalcValueRangeCast = ValueRangeBase::CPtr (*)(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType,
            ValueRangeBase::CPtr sourceRange,
            PracticalSemanticAnalyzer::StaticType::CPtr destType);

    // Members
    std::unordered_map< std::string, StaticTypeImpl::CPtr > types;
    const LookupContext *parent = nullptr;

    std::unordered_map< String, Symbol > symbols;
    std::unordered_map<
            PracticalSemanticAnalyzer::StaticType::CPtr,
            std::unordered_map<
                    PracticalSemanticAnalyzer::StaticType::CPtr,
                    CodeGenCast
            >
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

    void addFunctionPass1( const Tokenizer::Token *token );
    void addFunctionPass2( const Tokenizer::Token *token, StaticTypeImpl::CPtr type );

    void addLocalVar( const Tokenizer::Token *token, StaticTypeImpl::CPtr type, ExpressionId lvalue );

    const Symbol *lookupSymbol( String name ) const;

    void addCast(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            CodeGenCast codeGenCast, bool implicitAllowed );

    CodeGenCast lookupCast(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            bool implicit ) const;
};

} // End namespace AST

#endif // AST_LOOKUP_CONTEXT_H
