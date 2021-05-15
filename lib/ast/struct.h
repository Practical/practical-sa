/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2021 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_STRUCT_H
#define AST_STRUCT_H

#include "parser/struct.h"

#include <practical/practical.h>

namespace AST {

class LookupContext;
struct DelayedDefinitions;

class StructTypeImpl final : public PracticalSemanticAnalyzer::StaticType::Struct {
public:
    using Ptr = boost::intrusive_ptr<StructTypeImpl>;
    using CPtr = boost::intrusive_ptr<const StructTypeImpl>;

    explicit StructTypeImpl(String name, const LookupContext *parentCtx);
    StructTypeImpl( StructTypeImpl &&that );

    ~StructTypeImpl();

    virtual String getName() const override;
    virtual size_t getNumMembers() const override;
    virtual MemberDescriptor getMember( size_t index ) const override;

    virtual size_t getSize() const override;
    virtual size_t getAlignment() const override;

    void getMangledName(std::ostringstream &formatter) const;

    void definitionPass1( const NonTerminals::StructDef &parserStruct );
    bool definitionPass2( const NonTerminals::StructDef &parserStruct, DelayedDefinitions &delayedDefs );

private:
    // Members
    std::string _name;
    std::unique_ptr<LookupContext> _context;
    std::vector<String> _members;
    size_t _size = 0;
    size_t _alignment = 0;
};

} // namespace AST

#endif // AST_STRUCT_H
