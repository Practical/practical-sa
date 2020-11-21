/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "static_type.h"

#include "ast/pointers.h"

#include <sstream>

using namespace PracticalSemanticAnalyzer;

namespace AST {

ScalarTypeImpl::ScalarTypeImpl(
        String name, String mangledName, size_t size, size_t alignment, Scalar::Type type,
        PracticalSemanticAnalyzer::TypeId backendType, unsigned literalWeight
) :
    PracticalSemanticAnalyzer::StaticType::Scalar( size, alignment, type, backendType, literalWeight ),
    name( sliceToString(name) ),
    mangledName( sliceToString(mangledName) )
{
}

FunctionTypeImpl::FunctionTypeImpl(
        boost::intrusive_ptr<const StaticTypeImpl> &&returnType,
        std::vector< boost::intrusive_ptr<const StaticTypeImpl> > &&argumentTypes
) :
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

PointerTypeImpl::PointerTypeImpl( boost::intrusive_ptr<const StaticTypeImpl> pointed ) :
    pointed( downCast( pointed->removeFlags(StaticType::Flags::Reference) ) )
{}

void PointerTypeImpl::getMangledName( std::ostringstream &formatter ) const {
    formatter<<"p";
    pointed->getMangledName( formatter );
}

PracticalSemanticAnalyzer::StaticType::CPtr PointerTypeImpl::getPointedType() const {
    return pointed;
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

        Types operator()( const PointerTypeImpl &ptr ) {
            return &ptr;
        }
    };

    return std::visit( Visitor{ ._this=this }, content );
}

String StaticTypeImpl::getMangledName() const {
    if( mangledName.empty() ) {
        std::ostringstream formatter;
        getMangledName(formatter);

        mangledName = std::move(formatter).str();
    }

    return mangledName;
}

void StaticTypeImpl::getMangledName( std::ostringstream &formatter ) const {
    Flags::Type flags = getFlags();
    if( (flags & Flags::Reference) != 0 ) {
        formatter<<'r';
        flags &= ~Flags::Reference;
    }
    if( (flags & Flags::Mutable) != 0 ) {
        formatter<<'m';
        flags &= ~Flags::Mutable;
    }
    ASSERT( flags==0 )<<"Unhandled type flags "<<flags;

    struct Visitor {
        std::ostringstream &formatter;

        void operator()( const Scalar *scalar ) {
            static_cast<const ScalarTypeImpl *>(scalar)->getMangledName(formatter);
        }

        void operator()( const Function *function ) {
            static_cast<const FunctionTypeImpl *>(function)->getMangledName(formatter);
        }

        void operator()( const Pointer *pointer ) {
            static_cast<const PointerTypeImpl *>(pointer)->getMangledName(formatter);
        }
    };

    std::visit( Visitor{ .formatter=formatter }, getType() );
}

void FunctionTypeImpl::getMangledName(std::ostringstream &formatter) const {
    // Return value
    formatter<<"R"<<getReturnType()->getMangledName()<<"E";
    // Parameters
    formatter<<"P";
    for( unsigned i=0; i<getNumArguments(); ++i ) {
        formatter<<getArgumentType(i)->getMangledName();
    }
    formatter<<"E";
}

StaticTypeImpl::StaticTypeImpl( const StaticTypeImpl &that ) :
    valueRange( that.valueRange ),
    flags( that.flags )
{
    struct Visitor {
        StaticTypeImpl *_this;

        void operator()( const std::unique_ptr<ScalarTypeImpl> &scalar ) {
            _this->content = std::make_unique<ScalarTypeImpl>( *scalar );
        }

        void operator()( const std::unique_ptr<FunctionTypeImpl> &function ) {
            _this->content = std::make_unique<FunctionTypeImpl>( *function );
        }

        void operator()( const PointerTypeImpl &pointer ) {
            _this->content = pointer;
        }
    };

    std::visit( Visitor{ ._this=this }, that.content );
}

StaticTypeImpl::StaticTypeImpl( ScalarTypeImpl &&scalar, ValueRangeBase::CPtr valueRange ) :
    content( std::unique_ptr<ScalarTypeImpl>( new ScalarTypeImpl( std::move(scalar) ) ) ),
    valueRange(valueRange)
{
}

StaticTypeImpl::StaticTypeImpl( FunctionTypeImpl &&function ) :
    content( safenew<FunctionTypeImpl>( std::move(function) ) )
{
}

StaticTypeImpl::StaticTypeImpl( PointerTypeImpl &&ptr ) :
    valueRange(
            new PointerValueRange(
                downCast(ptr.getPointedType())->defaultRange(),
                BoolValueRange(true, true)
            )
        )
{
    // Easier to initialize content after the value range
    content = std::move(ptr);
}

std::ostream &operator<<( std::ostream &out, const AST::StaticTypeImpl::CPtr &type )
{
    return out<<static_cast<PracticalSemanticAnalyzer::StaticType::CPtr>(type);
}

StaticTypeImpl::CPtr downCast( StaticType::CPtr ptr ) {
    auto downCasted = dynamic_cast<const StaticTypeImpl *>( ptr.get() );
    ASSERT( downCasted || !ptr );

    return downCasted;
}

const PointerTypeImpl *downCast( const PracticalSemanticAnalyzer::StaticType::Pointer * ptr ) {
    ASSERT( ptr );
    auto downCasted = dynamic_cast<const PointerTypeImpl *>( ptr );
    ASSERT( downCasted );

    return downCasted;
}

const FunctionTypeImpl *downCast( const PracticalSemanticAnalyzer::StaticType::Function * ptr ) {
    ASSERT( ptr );
    auto downCasted = dynamic_cast<const FunctionTypeImpl *>( ptr );
    ASSERT( downCasted );

    return downCasted;
}

} // End namespace AST
