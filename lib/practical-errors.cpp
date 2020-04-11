/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "practical-errors.h"

#include "tokenizer.h"

#include <sstream>

namespace PracticalSemanticAnalyzer {

CastNotAllowed::CastNotAllowed(StaticType::CPtr src, StaticType::CPtr dst, bool implicit, size_t line, size_t col)
        : compile_error(line, col)
{
    std::stringstream buf;

    if( implicit )
        buf<<"Cannot implicitly convert from "<<src<<" to "<<dst;
    else
        buf<<"Cannot cast from "<<src<<" to "<<dst;

    setMsg( buf.str().c_str() );
}

IncompatibleTypes::IncompatibleTypes(StaticType::CPtr left, StaticType::CPtr right, size_t line, size_t col)
        : compile_error(line, col)
{
    std::stringstream buf;

    buf<<"Cannot find common type for "<<left<<" and "<<right;

    setMsg( buf.str().c_str() );
}

SymbolRedefined::SymbolRedefined(String symbol, size_t line, size_t col)
    : compile_error(line, col)
{
    std::stringstream buf;

    buf<<"Symbol "<<symbol<<" redefined";

    setMsg( buf.str().c_str() );
}

SymbolNotFound::SymbolNotFound(String symbol, size_t line, size_t col)
    : compile_error(line, col)
{
    std::stringstream buf;

    buf<<"Symbol "<<symbol<<" not found";

    setMsg( buf.str().c_str() );
}

CannotTakeValueOfFunction::CannotTakeValueOfFunction(const Tokenizer::Token *identifier) :
    compile_error(identifier->line, identifier->col)
{
    std::stringstream buf;

    buf<<"Trying to evaluate "<<identifier->text<<" which is of a callable type";

    setMsg( buf.str().c_str() );
}

TryToCallNonCallable::TryToCallNonCallable(const Tokenizer::Token *identifier) :
    compile_error(identifier->line, identifier->col)
{
    std::stringstream buf;

    buf<<"Trying to call "<<identifier->text<<" which is not of a callable type";

    setMsg( buf.str().c_str() );
}

} // namespace PracticalSemanticAnalyzer
