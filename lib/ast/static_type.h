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

#include <practical-sa.h>

namespace AST {

class BuiltinType {
    std::string name;

public:

    String getName() const;
};

class StaticTypeImpl : public PracticalSemanticAnalyzer::StaticType {
    private:
        String name;

    public:
        using CPtr = boost::intrusive_ptr<const StaticType>;
        using Ptr = boost::intrusive_ptr<StaticType>;

        explicit StaticTypeImpl( BuiltinType &&builtin );

        virtual String getName() const override final
        {
            return name;
        }

        static Ptr allocate( const char *name );

    private:
        explicit StaticTypeImpl(const char *name) :
            name(name)
        {}
};

} // End namespace AST

#endif // AST_STATIC_TYPE_H
