/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_CAST_CHAIN_H
#define AST_CAST_CHAIN_H

#include "ast/expression/expression_metadata.h"
#include "ast/lookup_context.h"
#include "ast/static_type.h"

namespace AST {

class CastChain {
    std::unique_ptr<CastChain> previousCast;
    const LookupContext::CastDescriptor &cast;
    ExpressionImpl::ExpressionMetadata metadata;

public:

    static std::unique_ptr<CastChain> allocate(
            const LookupContext &lookupContext,
            StaticTypeImpl::CPtr destinationType,
            const ExpressionImpl::ExpressionMetadata &srcMetadata,
            unsigned &weight,
            unsigned weightLimit );

    ExpressionId codeGen(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::FunctionGen *functionGen
        ) const;

    const ExpressionImpl::ExpressionMetadata getMetadata() const {
        return metadata;
    }
};

} // namespace AST

#endif // AST_CAST_CHAIN_H
