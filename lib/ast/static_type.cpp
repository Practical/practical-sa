/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "static_type.h"

#include "ast/arrays.h"
#include "ast/hash_modifiers.h"
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
        Types operator()( const std::unique_ptr<ScalarTypeImpl> &scalar ) {
            return scalar.get();
        }

        Types operator()( const std::unique_ptr<FunctionTypeImpl> &function ) {
            return function.get();
        }

        Types operator()( const PointerTypeImpl &ptr ) {
            return &ptr;
        }

        Types operator()( const ArrayTypeImpl &array ) {
            return &array;
        }

        Types operator()( StructTypeImpl::CPtr strct ) {
            return strct.get();
        }
    };

    return std::visit( Visitor{}, content );
}

StaticTypeImpl::CPtr StaticTypeImpl::coreType() const {
    struct Visitor {
        const StaticTypeImpl *_this;

        CPtr operator()( const std::unique_ptr<ScalarTypeImpl> &scalar ) {
            return downCast( _this->setFlags( 0 ) );
        }

        CPtr operator()( const std::unique_ptr<FunctionTypeImpl> &function ) {
            return downCast( _this->setFlags( 0 ) );
        }

        CPtr operator()( const PointerTypeImpl &ptr ) {
            return downCast( ptr.getPointedType() )->coreType();
        }

        CPtr operator()( const ArrayTypeImpl &array ) {
            return downCast( array.getElementType() )->coreType();
        }

        CPtr operator()( StructTypeImpl::CPtr strct ) {
            return downCast( _this->setFlags( 0 ) );
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
            downCast(function)->getMangledName(formatter);
        }

        void operator()( const Pointer *pointer ) {
            downCast(pointer)->getMangledName(formatter);
        }

        void operator()( const Array *array ) {
            downCast(array)->getMangledName(formatter);
        }

        void operator()( const Struct *strct ) {
            downCast(strct)->getMangledName(formatter);
        }
    };

    std::visit( Visitor{ .formatter=formatter }, getType() );
}

bool StaticTypeImpl::sizeKnown() const {
    Flags::Type flags = getFlags();
    if( (flags & Flags::Reference) != 0 )
        return true;

    struct Visitor {
        size_t operator()( const Scalar *scalar ) {
            return true;
        }
        size_t operator()( const Function *function ) {
            ABORT()<<"Trying to get sizeof(function)";
        }
        size_t operator()( const Pointer *pointer ) {
            return true;
        }
        size_t operator()( const Array *array ) {
            return downCast( array->getElementType() )->sizeKnown();
        }
        size_t operator()( const Struct *strct ) {
            return strct->getSize()!=0;
        }
    };

    return std::visit( Visitor{}, getType() );
}

size_t StaticTypeImpl::getSize() const {
    Flags::Type flags = getFlags();
    if( (flags & Flags::Reference) != 0 )
        return ptrSize();

    struct Visitor {
        size_t operator()( const Scalar *scalar ) {
            return scalar->getSize();
        }
        size_t operator()( const Function *function ) {
            ABORT()<<"Trying to get sizeof(function)";
        }
        size_t operator()( const Pointer *pointer ) {
            return ptrSize();
        }
        size_t operator()( const Array *array ) {
            return array->getSize();
        }
        size_t operator()( const Struct *strct ) {
            return strct->getSize();
        }
    };

    return std::visit( Visitor{}, getType() );
}

size_t StaticTypeImpl::getAlignment() const {
    Flags::Type flags = getFlags();
    if( (flags & Flags::Reference) != 0 )
        return ptrAlignment();

    struct Visitor {
        size_t operator()( const Scalar *scalar ) {
            return scalar->getAlignment();
        }
        size_t operator()( const Function *function ) {
            ABORT()<<"Trying to get alignof(function)";
        }
        size_t operator()( const Pointer *pointer ) {
            return ptrAlignment();
        }
        size_t operator()( const Array *array ) {
            return array->getAlignment();
        }
        size_t operator()( const Struct *strct ) {
            return downCast( strct )->getAlignment();
        }
    };

    return std::visit( Visitor{}, getType() );
}

size_t StaticTypeImpl::getHash() const {
    auto typeType = getType();

    struct Visitor {
        const StaticTypeImpl &_this;

        size_t operator()( const StaticType::Scalar *scalar ) {
            return std::hash<String>{}( scalar->getName() );
        }

        size_t operator()( const StaticType::Function *function ) {
            size_t result = 0;

            auto numArguments = function->getNumArguments();
            for( unsigned i=0; i<numArguments; ++i ) {
                result += downCast( function->getArgumentType(i) )->getHash();
                result *= FibonacciHashMultiplier;
            }

            result += downCast( function->getReturnType() )->getHash();

            return result;
        }

        size_t operator()( const StaticType::Pointer *pointer ) {
            return
                    downCast( pointer->getPointedType() )->getHash() *
                    (FibonacciHashMultiplier - PointerModifier);
        }

        size_t operator()( const StaticType::Array *array ) {
            return downCast( array->getElementType() )->getHash() * (FibonacciHashMultiplier - ArrayModifier) +
                   array->getNumElements();
        }

        size_t operator()( const StaticType::Struct *strct ) {
            return downCast( strct )->getHash();
        }
    };

    size_t retVal =
            typeType.index() * FibonacciHashMultiplier + std::visit( Visitor{._this = *this}, typeType );

    size_t asserter = 0;
    if( (getFlags() & Flags::Reference) != 0 ) {
        retVal *= FibonacciHashMultiplier-ReferenceModifier;
        asserter |= StaticType::Flags::Reference;
    }
    if( (getFlags() & Flags::Mutable) != 0 ) {
        retVal *= FibonacciHashMultiplier-MutableModifier;
        asserter |= StaticType::Flags::Mutable;
    }
    ASSERT( getFlags() == asserter )<<"Unhandled type flag. Flags "<<getFlags()<<", handled "<<asserter;

    return retVal;
}

size_t StaticTypeImpl::calcHashInternal( const StructTypeImpl *anchor ) const {
    struct Visitor {
        const StructTypeImpl *anchor;
        const StaticTypeImpl &this_;

        size_t operator()( const Scalar *scalar ) const {
            return this_.getHash();
        }

        size_t operator()( const Function *function ) const {
            return this_.getHash();
        }

        size_t operator()( const Pointer *pointer ) const {
            return
                    downCast( pointer->getPointedType() )->calcHashInternal( anchor ) *
                    (FibonacciHashMultiplier - PointerModifier);
        }

        size_t operator()( const Array *array ) const {
            return
                    downCast( array->getElementType() )->calcHashInternal( anchor ) *
                    (FibonacciHashMultiplier - ArrayModifier) +
                    array->getNumElements();
        }

        size_t operator()( const Struct *strct ) const {
            return downCast( strct )->calcHash( anchor );
        }
    };

    return std::visit( Visitor{.anchor = anchor, .this_ = *this}, getType() );
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

ArrayTypeImpl::ArrayTypeImpl( boost::intrusive_ptr<const StaticTypeImpl> elementType, size_t dimension ) :
    elementType(elementType), dimension(dimension)
{}

void ArrayTypeImpl::getMangledName(std::ostringstream &formatter) const {
    formatter<<"A"<<getNumElements();

    downCast(getElementType())->getMangledName(formatter);
}

PracticalSemanticAnalyzer::StaticType::CPtr ArrayTypeImpl::getElementType() const {
    return elementType;
}

size_t ArrayTypeImpl::getNumElements() const {
    return dimension;
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

        void operator()( const ArrayTypeImpl &array ) {
            _this->content = array;
        }

        void operator()( StructTypeImpl::Ptr strct ) {
            _this->content = strct;
        }

        void operator()( StructTypeImpl::CPtr strct ) {
            _this->content = strct;
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

StaticTypeImpl::StaticTypeImpl( ArrayTypeImpl &&ptr ) :
    valueRange(
            new ArrayValueRange(
                downCast(ptr.getElementType())->defaultRange(),
                ptr.getNumElements()
            )
        )
{
    // Easier to initialize content after the value range
    content = std::move(ptr);
}

StaticTypeImpl::StaticTypeImpl( StructTypeImpl &&strct ) :
    content( StructTypeImpl::Ptr(new StructTypeImpl( std::move(strct) )) )
{}

void StaticTypeImpl::completeConstruction() {
    StructTypeImpl::CPtr typeUnderConstruction( getMutableStruct() );
    content = typeUnderConstruction;
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

const ArrayTypeImpl *downCast( const PracticalSemanticAnalyzer::StaticType::Array * ptr ) {
    ASSERT( ptr );
    auto downCasted = dynamic_cast<const ArrayTypeImpl *>( ptr );
    ASSERT( downCasted );

    return downCasted;
}

const StructTypeImpl *downCast( const PracticalSemanticAnalyzer::StaticType::Struct * ptr ) {
    ASSERT( ptr );
    auto downCasted = dynamic_cast<const StructTypeImpl *>( ptr );
    ASSERT( downCasted );

    return downCasted;
}

size_t alignUp( size_t ptr, size_t alignment ) {
    ASSERT( alignment>0 );
    ptr += alignment - 1;
    ptr -= ptr % alignment;

    return ptr;
}

} // End namespace AST
