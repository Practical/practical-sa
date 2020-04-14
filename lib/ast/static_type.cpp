/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "static_type.h"

using namespace PracticalSemanticAnalyzer;

namespace AST {

ScalarImpl::ScalarImpl( String name, size_t size, size_t alignment, PracticalSemanticAnalyzer::TypeId backendType ) :
    PracticalSemanticAnalyzer::StaticType::Scalar( size, alignment, backendType ),
    name( sliceToString(name) )
{
}

StaticType::Types StaticTypeImpl::getType() const {
    struct Visitor {
        const StaticType *_this;

        Types operator()( const std::unique_ptr<ScalarImpl> &scalar ) {
            return scalar.get();
        }
    };

    return std::visit( Visitor{ ._this=this }, content );
}

StaticTypeImpl::StaticTypeImpl( ScalarImpl &&scalar ) :
    content( std::unique_ptr<ScalarImpl>( new ScalarImpl( std::move(scalar) ) ) )
{
}

} // End namespace AST
