/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "config.h"

#include "ast/ast.h"
#include "ast/static_type.h"
#include "mmap.h"
#include "parser.h"

#include <practical/defines.h>
#include <practical/practical.h>

DEF_TYPED_NS( PracticalSemanticAnalyzer, ModuleId );
DEF_TYPED_NS( PracticalSemanticAnalyzer, ExpressionId );
DEF_TYPED_NS( PracticalSemanticAnalyzer, JumpPointId );

namespace PracticalSemanticAnalyzer {

std::ostream &operator<<(std::ostream &out, StaticType::Scalar::Type type) {
#define CASE(c) case StaticType::Scalar::Type::c: return out<<#c
    switch(type) {
        CASE(Void);
        CASE(Bool);
        CASE(SignedInt);
        CASE(UnsignedInt);
        CASE(Char);
    }
#undef CASE

    return out<<"Unkown type "<<static_cast<unsigned>(type);
}

bool StaticType::Scalar::operator==( const Scalar &rhs ) const {
    return
            size==rhs.size &&
            alignment==rhs.alignment &&
            type==rhs.type;
}

bool StaticType::Function::operator==( const Function &rhs ) const {
    if( getReturnType() != rhs.getReturnType() )
        return false;
    if( getNumArguments() != rhs.getNumArguments() )
        return false;

    for( size_t argumentIdx=0; argumentIdx<getNumArguments(); ++argumentIdx ) {
        if( getArgumentType(argumentIdx) != rhs.getArgumentType(argumentIdx) )
            return false;
    }

    return true;
}

bool StaticType::Pointer::operator==( const Pointer &rhs ) const {
    return getPointedType() == rhs.getPointedType();
}

bool StaticType::operator==( const StaticType &rhs ) const {
    auto leftType = getType();
    auto rightType = rhs.getType();

    if( getFlags()!=rhs.getFlags() )
        return false;

    if( leftType.index() != rightType.index() )
        return false;

    struct Visitor {
        StaticType::Types rightTypes;

        bool operator()( const Scalar *scalar ) {
            return (*scalar)==*( std::get<const Scalar *>( rightTypes ) );
        }

        bool operator()( const StaticType::Function *function ) {
            return (*function)==*( std::get<const Function *>( rightTypes ) );
        }

        bool operator()( const StaticType::Pointer *pointer ) {
            return (*pointer)==*( std::get<const Pointer *>( rightTypes ) );
        }
    };

    return std::visit( Visitor{ .rightTypes = rhs.getType() }, getType() );
}

std::ostream &operator<<(std::ostream &out, StaticType::CPtr type) {
    if( !type ) {
        return out<<"Type(nullptr)";
    }

    struct Visitor {
        std::ostream &out;

        void operator()( const StaticType::Scalar *scalar ) {
            out<<scalar->getName();
        }

        void operator()( const StaticType::Function *function ) {
            out<<"(";
            for( unsigned argNum=0; argNum<function->getNumArguments(); argNum++ ) {
                if( argNum!=0 )
                    out<<",";
                out<<function->getArgumentType(argNum);
            }
            out<<")->"<<function->getReturnType();
        }

        void operator()( const StaticType::Pointer *pointer ) {
            out << pointer->getPointedType() << "@";
        }
    };

    std::visit( Visitor{ .out = out }, type->getType() );

    StaticType::Flags::Type flags = type->getFlags();
    if( flags & StaticType::Flags::Mutable ) {
        out << " mut";
        flags &= ~StaticType::Flags::Mutable;
    }
    if( flags & StaticType::Flags::Reference ) {
        out << " ref";
        flags &= ~StaticType::Flags::Reference;
    }

    return out;
}

std::unique_ptr<CompilerArguments> allocateArguments() {
    return safenew<CompilerArguments>();
}

void prepare( BuiltinContextGen *ctxGen ) {
    AST::AST::prepare(ctxGen);
}

int compile(std::string path, const CompilerArguments *arguments, ModuleGen *codeGen) {
    // Load file into memory
    Mmap<MapMode::ReadOnly> sourceFile(path);

    AST::AST ast;

    // Parse + symbols lookup
    ASSERT( AST::AST::prepared() )<<"compile called without calling prepare first";
    auto tokenizedModule = Tokenizer::Tokenizer::tokenize( sourceFile.getSlice<const char>() );
    NonTerminals::Module module;
    module.parse( tokenizedModule );

    // And that other thing
    ast.codeGen( module, codeGen );

    return 0;
}

} // PracticalSemanticAnalyzer

using namespace PracticalSemanticAnalyzer;

namespace std {

size_t hash< StaticType >::operator()(const StaticType &type) const {
    static constexpr size_t PointerModifier = 4;
    static constexpr size_t ReferenceModifier = 6;
    static constexpr size_t MutableModifier = 10;

    auto typeType = type.getType();

    struct Visitor {
        size_t operator()( const StaticType::Scalar *scalar ) {
            return hash<String>{}( scalar->getName() );
        }

        size_t operator()( const StaticType::Function *function ) {
            size_t result = 0;

            auto numArguments = function->getNumArguments();
            for( unsigned i=0; i<numArguments; ++i ) {
                result += hash{}( *function->getArgumentType(i) );
                result *= FibonacciHashMultiplier;
            }

            result += hash{}( *function->getReturnType() );

            return result;
        }

        size_t operator()( const StaticType::Pointer *pointer ) {
            return hash{}( *pointer->getPointedType() ) * (FibonacciHashMultiplier - PointerModifier);
        }
    };

    size_t retVal = typeType.index() * FibonacciHashMultiplier + std::visit( Visitor{}, typeType );

    size_t asserter = 0;
    if( (type.getFlags() & StaticType::Flags::Reference) != 0 ) {
        retVal *= FibonacciHashMultiplier-ReferenceModifier;
        asserter |= StaticType::Flags::Reference;
    }
    if( (type.getFlags() & StaticType::Flags::Mutable) != 0 ) {
        retVal *= FibonacciHashMultiplier-MutableModifier;
        asserter |= StaticType::Flags::Mutable;
    }
    ASSERT( type.getFlags() == asserter )<<"Unhandled type flag. Flags "<<type.getFlags()<<", handled "<<asserter;

    return retVal;
}

} // namespace std
