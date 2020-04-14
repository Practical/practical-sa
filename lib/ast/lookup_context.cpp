/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "lookup_context.h"

#include <asserts.h>

using namespace PracticalSemanticAnalyzer;

namespace AST {

StaticTypeImpl::CPtr LookupContext::lookupType( String name ) const {
    auto iter = types.find(name);

    if( iter==types.end() )
        return StaticTypeImpl::CPtr();

    return iter->second;
}

StaticTypeImpl::CPtr LookupContext::registerScalarType( ScalarImpl &&type ) {
    auto iter = types.emplace(
            type.getName(),
            StaticTypeImpl::allocate( std::move(type) ) );
    ASSERT( iter.second )<<"registerBuiltinType called on "<<type.getName()<<" which is already registered";

    return iter.first->second;
}

} // End namespace AST
