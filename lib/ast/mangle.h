/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_MANGLE_H
#define AST_MANGLE_H

#include <practical/practical.h>

#include <string>

namespace AST {

std::string getFunctionMangledName( String baseName, PracticalSemanticAnalyzer::StaticType::CPtr type );
std::string getMangledSymbol( String symbol );

} // namespace AST

#endif // AST_MANGLE_H
