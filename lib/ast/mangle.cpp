/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/mangle.h"

#include <sstream>

namespace AST {

#define MANGLE_PREFIX "_P"

static void genMangledSymbol( std::stringstream &formatter, String symbolName ) {
    formatter<<symbolName.size()<<symbolName;
}

std::string getFunctionMangledName( String baseName, PracticalSemanticAnalyzer::StaticType::CPtr type ) {
    // TODO To allow runnable programs
    if( baseName==String("main") )
        return sliceToString(baseName);

    std::stringstream formatter;

    formatter<<MANGLE_PREFIX;
    genMangledSymbol( formatter, baseName );
    formatter<<type->getMangledName();

    return std::move(formatter).str();
}

std::string getMangledSymbol( String symbolName ) {
    std::stringstream formatter;
    formatter<<MANGLE_PREFIX;
    genMangledSymbol( formatter, symbolName );

    return std::move(formatter).str();
}

} // namespace AST
