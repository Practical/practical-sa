#include "practical-errors.h"

#include <sstream>

namespace PracticalSemanticAnalyzer {

ImplicitCastNotAllowed::ImplicitCastNotAllowed(const StaticType *src, const StaticType *dst, size_t line, size_t col)
        : compile_error(line, col)
{
    std::stringstream buf;

    buf<<"Cannot implicitly cast from "<<*src<<" to "<<*dst;

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

} // namespace PracticalSemanticAnalyzer
