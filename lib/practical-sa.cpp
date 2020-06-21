/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "config.h"

#include <practical-sa.h>

#include "ast/ast.h"
#include "ast/static_type.h"
#include "mmap.h"
#include "parser.h"

#include "defines.h"

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
    ABORT()<<"TODO implement";
}

String StaticType::getMangledName() const {
    struct Visitor {
        String operator()( const Function *function ) {
            return function->getMangledName();
        }

        String operator()( const Scalar *scalar ) {
            return scalar->getMangledName();
        }
    };

    return std::visit( Visitor{}, getType() );
}

bool StaticType::operator==( const StaticType &rhs ) const {
    auto leftType = getType();
    auto rightType = rhs.getType();

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
    };

    return std::visit( Visitor{ .rightTypes = rhs.getType() }, getType() );
}

std::ostream &operator<<(std::ostream &out, StaticType::CPtr type) {
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
    };

    std::visit( Visitor{ .out = out }, type->getType() );

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
    };

    return typeType.index() * FibonacciHashMultiplier + std::visit( Visitor{}, typeType );
}

} // namespace std
