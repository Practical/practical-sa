/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_MODULE_H
#define AST_MODULE_H

#include "ast/lookup_context.h"
#include "parser.h"

namespace AST {

class Module final : public boost::intrusive_ref_counter<Module, boost::thread_unsafe_counter>, private NoCopy {
    const NonTerminals::Module &parserModule;
    LookupContext lookupContext;
    PracticalSemanticAnalyzer::ModuleId moduleId;

public:
    using Ptr = boost::intrusive_ptr<Module>;

    explicit Module( const NonTerminals::Module &parserModule, const LookupContext &parentLookupContext );

    void symbolsPass1();
    void symbolsPass2();
    void codeGen( PracticalSemanticAnalyzer::ModuleGen *codeGen );
};

} // End namespace AST

#endif // AST_MODULE_H
