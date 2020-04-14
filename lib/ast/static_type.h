/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_STATIC_TYPE_H
#define AST_STATIC_TYPE_H

#include "asserts.h"

#include <practical-sa.h>

#include <memory>

namespace AST {

class ScalarImpl final : public PracticalSemanticAnalyzer::StaticType::Scalar, private NoCopy {
    std::string name;

public:
    explicit ScalarImpl( String name, size_t size, size_t alignment, PracticalSemanticAnalyzer::TypeId backendType );
    ScalarImpl( ScalarImpl &&that ) :
        PracticalSemanticAnalyzer::StaticType::Scalar( that ),
        name( std::move(that.name) )
    {}

    virtual String getName() const override final {
        return name.c_str();
    }
};

class StaticTypeImpl : public PracticalSemanticAnalyzer::StaticType {
private:
    std::variant<
            std::unique_ptr<ScalarImpl>
    > content;

public:
    using CPtr = boost::intrusive_ptr<const StaticType>;
    using Ptr = boost::intrusive_ptr<StaticType>;

    template<typename... Args>
    static Ptr allocate(Args&&... args) {
        return new StaticTypeImpl( std::forward<Args>(args)... );
    }

    virtual Types getType() const override final;

private:
    explicit StaticTypeImpl( ScalarImpl &&scalar );
};

} // End namespace AST

#endif // AST_STATIC_TYPE_H
