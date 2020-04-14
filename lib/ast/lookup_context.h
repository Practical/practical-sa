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
#include "slice.h"

#include <string>
#include <unordered_map>

namespace AST {

class LookupContext {
    std::unordered_map< std::string, StaticTypeImpl::CPtr > types;

public:
    StaticTypeImpl::CPtr lookupType( String name ) const;

    StaticTypeImpl::CPtr registerScalarType( ScalarImpl &&type );
};

} // End namespace AST

#endif // AST_LOOKUP_CONTEXT_H
