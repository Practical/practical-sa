/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPECTED_RESULT_H
#define AST_EXPECTED_RESULT_H

#include <practical-sa.h>

namespace AST {

class ExpectedResult {
    PracticalSemanticAnalyzer::StaticType::CPtr type;
    bool mandatory = false;

public:
    ExpectedResult() = default;
    /* implicit */ ExpectedResult( PracticalSemanticAnalyzer::StaticType::CPtr type, bool mandatory = true );
};

} // namespace AST

#endif // AST_EXPECTED_RESULT_H
