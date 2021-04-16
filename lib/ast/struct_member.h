/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2021 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_STRUCT_MEMBER_H
#define AST_STRUCT_MEMBER_H

#include "tokenizer.h"

#include <boost/intrusive_ptr.hpp>

namespace AST {

class StaticTypeImpl;

struct StructMember {
    const Tokenizer::Token *token;
    boost::intrusive_ptr<const StaticTypeImpl> type;
    size_t offset;

    StructMember(const Tokenizer::Token *token, boost::intrusive_ptr<const StaticTypeImpl> type, size_t offset) :
        token(token),
        type(std::move(type)),
        offset(offset)
    {}
};

} // namespace AST

#endif // AST_STRUCT_MEMBER_H
