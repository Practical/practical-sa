/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_FUNCTION_H
#define AST_FUNCTION_H

#include "ast/lookup_context.h"
#include "parser.h"

namespace AST {

class Function : private NoCopy {
    const NonTerminals::FuncDef &parserFunction;
    String name;
    LookupContext lookupCtx;
    StaticType::CPtr returnType;
    std::vector< PracticalSemanticAnalyzer::ArgumentDeclaration > arguments;

public:
    explicit Function( const NonTerminals::FuncDef &parserFunction, const LookupContext &parentCtx );

    void codeGen( std::shared_ptr<PracticalSemanticAnalyzer::FunctionGen> functionGen );
};

} // namespace AST

#endif // AST_FUNCTION_H
