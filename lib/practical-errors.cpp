/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "tokenizer.h"

#include <practical/errors.h>

#include <sstream>

namespace PracticalSemanticAnalyzer {

IncompatibleTypes::IncompatibleTypes(
        StaticType::CPtr left, StaticType::CPtr right, const SourceLocation &location) :
    compile_error(location)
{
    std::stringstream buf;

    buf<<"Cannot find common type for "<<left<<" and "<<right;

    setMsg( buf.str().c_str() );
}

SymbolRedefined::SymbolRedefined(String symbol, const SourceLocation &location)
    : compile_error(location)
{
    std::stringstream buf;

    buf<<"Symbol "<<symbol<<" redefined";

    setMsg( buf.str().c_str() );
}

SymbolNotFound::SymbolNotFound(String symbol, const SourceLocation &location)
    : compile_error(location)
{
    std::stringstream buf;

    buf<<"Symbol "<<symbol<<" not found";

    setMsg( buf.str().c_str() );
}

CannotTakeValueOfFunction::CannotTakeValueOfFunction(const Tokenizer::Token *identifier) :
    compile_error(identifier->location)
{
    std::stringstream buf;

    buf<<"Trying to evaluate "<<identifier->text<<" which is of a callable type";

    setMsg( buf.str().c_str() );
}

TryToCallNonCallable::TryToCallNonCallable(const Tokenizer::Token *identifier) :
    compile_error(identifier->location)
{
    std::stringstream buf;

    buf<<"Trying to call "<<identifier->text<<" which is not of a callable type";

    setMsg( buf.str().c_str() );
}

NoMatchingOverload::NoMatchingOverload(const Tokenizer::Token *identifier) :
    compile_error(identifier->location)
{
    setMsg( "Trying to call function with no matching overload" );
}

AmbiguousOverloads::AmbiguousOverloads(const Tokenizer::Token *identifier) :
    compile_error(identifier->location)
{
    setMsg( "Trying to call function with ambiguous overload resolution" );
}

CastError::CastError(
        const char *msg, StaticType::CPtr src, StaticType::CPtr dst, bool implicit,
        const SourceLocation &location) :
    compile_error(location)
{
    std::stringstream buf;

    if( implicit )
        buf<<"Cannot implicitly convert from "<<src<<" to "<<dst<<": "<<msg;
    else
        buf<<"Cannot cast from "<<src<<" to "<<dst<<": msg";

    setMsg( buf.str().c_str() );
}


CastNotAllowed::CastNotAllowed(
        StaticType::CPtr src, StaticType::CPtr dst, bool implicit, const SourceLocation &location) :
    CastError( "No cast chain", src, dst, implicit, location )
{}

AmbiguousCast::AmbiguousCast(
        StaticType::CPtr src, StaticType::CPtr dst, bool implicit, const SourceLocation &location) :
    CastError( "Cast chain ambiguous", src, dst, implicit, location )
{
}

LValueRequired::LValueRequired(StaticType::CPtr wrongType, const SourceLocation &location) :
    compile_error( "LValue required", location )
{}

} // namespace PracticalSemanticAnalyzer
