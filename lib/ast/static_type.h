/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_STATIC_TYPE_H
#define AST_STATIC_TYPE_H

#include "ast/value_range_base.h"
#include "asserts.h"

#include <practical/practical.h>

#include <memory>
#include <sstream>

namespace AST {

class StaticTypeImpl;

class ScalarTypeImpl final : public PracticalSemanticAnalyzer::StaticType::Scalar {
    std::string name;
    std::string mangledName;

public:
    explicit ScalarTypeImpl(
            String name,
            String mangledName,
            size_t size,
            size_t alignment,
            Scalar::Type type,
            PracticalSemanticAnalyzer::TypeId backendType,
            unsigned literalWeight );
    ScalarTypeImpl( ScalarTypeImpl &&that ) :
        PracticalSemanticAnalyzer::StaticType::Scalar( that ),
        name( std::move(that.name) ),
        mangledName( std::move(that.mangledName) )
    {}
    ScalarTypeImpl( const ScalarTypeImpl &that ) = default;

    virtual String getName() const override {
        return name.c_str();
    }

    virtual void getMangledName(std::ostringstream &formatter) const {
        formatter<<mangledName;
    }
};

class FunctionTypeImpl final : public PracticalSemanticAnalyzer::StaticType::Function {
    boost::intrusive_ptr<const StaticTypeImpl> returnType;
    std::vector< boost::intrusive_ptr<const StaticTypeImpl> > argumentTypes;

public:
    explicit FunctionTypeImpl(
            boost::intrusive_ptr<const StaticTypeImpl> &&returnType,
            std::vector< boost::intrusive_ptr<const StaticTypeImpl> > &&argumentTypes);

    FunctionTypeImpl( FunctionTypeImpl &&that ) :
        returnType( std::move(that.returnType) ),
        argumentTypes( std::move(that.argumentTypes) )
    {}
    FunctionTypeImpl( const FunctionTypeImpl &that ) = default;

    PracticalSemanticAnalyzer::StaticType::CPtr getReturnType() const override;

    size_t getNumArguments() const override {
        return argumentTypes.size();
    }

    PracticalSemanticAnalyzer::StaticType::CPtr getArgumentType( unsigned index ) const override;

    void getMangledName(std::ostringstream &formatter) const;
};

class ArrayTypeImpl final : public PracticalSemanticAnalyzer::StaticType::Array {
    boost::intrusive_ptr<const StaticTypeImpl> elementType;
    size_t dimension;

public:
    explicit ArrayTypeImpl( boost::intrusive_ptr<const StaticTypeImpl> elementType, size_t dimension );

    void getMangledName(std::ostringstream &formatter) const;
    virtual PracticalSemanticAnalyzer::StaticType::CPtr getElementType() const override;
    virtual size_t getNumElements() const override;
};

class PointerTypeImpl final : public PracticalSemanticAnalyzer::StaticType::Pointer {
    boost::intrusive_ptr<const StaticTypeImpl> pointed;

public:
    explicit PointerTypeImpl( boost::intrusive_ptr<const StaticTypeImpl> pointed );

    void getMangledName(std::ostringstream &formatter) const;
    virtual PracticalSemanticAnalyzer::StaticType::CPtr getPointedType() const override;
};

class StaticTypeImpl final : public PracticalSemanticAnalyzer::StaticType {
private:
    std::variant<
            std::unique_ptr<ScalarTypeImpl>,
            std::unique_ptr<FunctionTypeImpl>,
            PointerTypeImpl,
            ArrayTypeImpl
    > content;
    ValueRangeBase::CPtr valueRange;
    mutable std::string mangledName;
    Flags::Type flags = 0;

public:
    using CPtr = boost::intrusive_ptr<const StaticTypeImpl>;
    using Ptr = boost::intrusive_ptr<StaticTypeImpl>;

    template<typename... Args>
    static Ptr allocate(Args&&... args) {
        return new StaticTypeImpl( std::forward<Args>(args)... );
    }

    virtual Types getType() const override final;

    virtual String getMangledName() const override;
    void getMangledName( std::ostringstream &formatter ) const;

    ValueRangeBase::CPtr defaultRange() const {
        return valueRange;
    }

    virtual Flags::Type getFlags() const override {
        return flags;
    }

    virtual StaticType::CPtr setFlags( Flags::Type newFlags ) const override {
        if( flags==newFlags )
            return this;

        Ptr ret = allocate( *this );
        ret->flags = newFlags;

        return ret;
    }

    friend std::ostream &operator<<( std::ostream &out, const AST::StaticTypeImpl::CPtr &type );

private:
    // Copy
    explicit StaticTypeImpl( const StaticTypeImpl &that );

    explicit StaticTypeImpl( ScalarTypeImpl &&scalar, ValueRangeBase::CPtr valueRange );
    explicit StaticTypeImpl( FunctionTypeImpl &&function );
    explicit StaticTypeImpl( ArrayTypeImpl &&array );
    explicit StaticTypeImpl( PointerTypeImpl &&ptr );
};

StaticTypeImpl::CPtr downCast( PracticalSemanticAnalyzer::StaticType::CPtr ptr );
const PointerTypeImpl *downCast( const PracticalSemanticAnalyzer::StaticType::Pointer * ptr );
const FunctionTypeImpl *downCast( const PracticalSemanticAnalyzer::StaticType::Function * ptr );
const ArrayTypeImpl *downCast( const PracticalSemanticAnalyzer::StaticType::Array * ptr );

} // End namespace AST

inline bool operator==( const AST::StaticTypeImpl::CPtr &lhs, const AST::StaticTypeImpl::CPtr &rhs ) {
    if( !lhs && !rhs )
        return true;

    if( !lhs || !rhs )
        return false;

    return *lhs == *rhs;
}

inline bool operator!=( const AST::StaticTypeImpl::CPtr &lhs, const AST::StaticTypeImpl::CPtr &rhs ) {
    return !( lhs == rhs );
}

inline std::ostream &operator<<( std::ostream &out, const AST::StaticTypeImpl &type ) {
    return out<<AST::StaticTypeImpl::CPtr(&type);
}

namespace std {
    template<>
    class hash< AST::StaticTypeImpl::CPtr > {
    public:
        size_t operator()( const AST::StaticTypeImpl::CPtr &ptr ) const {
            return hash< PracticalSemanticAnalyzer::StaticType::CPtr >()( ptr.get() );
        }
    };
}

#endif // AST_STATIC_TYPE_H
