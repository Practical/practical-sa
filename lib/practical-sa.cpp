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

#include "ast.h"
#include "mmap.h"
#include "parser.h"

#include "defines.h"

namespace PracticalSemanticAnalyzer {

StaticType &StaticType::operator=( StaticType &&that ) {
    ASSERT( id==TypeId(0) ) << "StaticType move operator called on initialized type";
    std::swap( id, that.id );

    return *this;
}
std::ostream &operator<<(std::ostream &out, const StaticType &type) {
    auto namedType = lookupTypeId(type.getId());

    out << namedType->name();

    return out;
}

std::unique_ptr<CompilerArguments> allocateArguments() {
    return safenew<CompilerArguments>();
}

int compile(std::string path, const CompilerArguments *arguments, ModuleGen *codeGen) {
    // Load file into memory
    Mmap<MapMode::ReadOnly> sourceFile(path);

    AST::AST ast;

    // Parse + symbols lookup
    ast.prepare();
    ast.parseModule(sourceFile.getSlice<const char>());

    // And that other thing
    ast.codeGen(codeGen);

    return 0;
}

const NamedType *lookupTypeId(TypeId id) {
    return LookupContext::lookupType(id);
}

} // PracticalSemanticAnalyzer
