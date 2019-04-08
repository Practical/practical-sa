/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef EXPECTED_TYPE_H
#define EXPECTED_TYPE_H

#include "practical-sa.h"

struct ExpectedType {
    PracticalSemanticAnalyzer::StaticType::Ptr type = nullptr;
    bool mandatory = false;

    ExpectedType() {}
    explicit ExpectedType( PracticalSemanticAnalyzer::StaticType::Ptr type, bool mandatory = true ) :
        type(type), mandatory(mandatory)
    {}

    operator bool() const {
        return type!=nullptr;
    }
private:
    operator unsigned short() const;
};

#endif // EXPECTED_TYPE_H
