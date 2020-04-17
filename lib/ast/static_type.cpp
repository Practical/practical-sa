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

ScalarTypeImpl::ScalarTypeImpl(
        String name, size_t size, size_t alignment, PracticalSemanticAnalyzer::TypeId backendType ) :
    PracticalSemanticAnalyzer::StaticType::Scalar( size, alignment, backendType ),
    name( sliceToString(name) )
{
}

FunctionTypeImpl::FunctionTypeImpl(
        boost::intrusive_ptr<const StaticTypeImpl> &&returnType,
        std::vector< boost::intrusive_ptr<const StaticTypeImpl> > &&argumentTypes ) :
    returnType( std::move(returnType) ),
    argumentTypes( std::move(argumentTypes) )
{
}

StaticType::CPtr FunctionTypeImpl::getReturnType() const {
    return returnType;
}

StaticType::CPtr FunctionTypeImpl::getArgumentType( unsigned index ) const {
    return argumentTypes[index];
}

StaticType::Types StaticTypeImpl::getType() const {
    struct Visitor {
        const StaticType *_this;

        Types operator()( const std::unique_ptr<ScalarTypeImpl> &scalar ) {
            return scalar.get();
        }

        Types operator()( const std::unique_ptr<FunctionTypeImpl> &function ) {
            return function.get();
        }
    };

    return std::visit( Visitor{ ._this=this }, content );
}

StaticTypeImpl::StaticTypeImpl( ScalarTypeImpl &&scalar ) :
    content( std::unique_ptr<ScalarTypeImpl>( new ScalarTypeImpl( std::move(scalar) ) ) )
{
}

StaticTypeImpl::StaticTypeImpl( FunctionTypeImpl &&function ) :
    content( safenew<FunctionTypeImpl>( std::move(function) ) )
{
}

} // End namespace AST
