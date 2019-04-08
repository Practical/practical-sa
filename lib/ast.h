/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_H
#define AST_H

#include "ast_nodes.h"
#include "lookup_context.h"
#include "parser.h"
#include "practical-sa.h"

#include <memory>

extern PracticalSemanticAnalyzer::ExpressionId::Allocator<> expressionIdAllocator;
extern PracticalSemanticAnalyzer::ExpressionId voidExpressionId;
extern PracticalSemanticAnalyzer::ModuleId::Allocator<> moduleIdAllocator;

namespace AST {

class AST {
    static LookupContext globalCtx;
    std::unordered_map<String, std::unique_ptr<NonTerminals::Module>> parsedModules;
    std::unordered_map<String, std::unique_ptr<::AST::Module>> modulesAST;

public:

    AST() {
    }

    static const LookupContext &getGlobalCtx() {
        return globalCtx;
    }
    static StaticType::Ptr deductLiteralRange(LongEnoughInt value);

    void prepare();

    void parseModule(String moduleSource);

    void codeGen(PracticalSemanticAnalyzer::ModuleGen *codeGen);
};

} // namespace AST

#endif // AST_H
