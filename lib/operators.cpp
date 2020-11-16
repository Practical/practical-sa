/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "operators.h"

#include "asserts.h"

using namespace Tokenizer;

namespace Operators {

const std::vector< OperatorPriority > operators = {
    {
        // 1
        .kind = OperatorPriority::OpKind::Infix,
        .operators = {
            { Tokens::OP_DOUBLE_COLON, OperatorType::Regular }
        }
    },
    {
        // 2
        .kind = OperatorPriority::OpKind::Postfix,
        .operators = {
            { Tokens::OP_PLUS_PLUS, OperatorType::Regular },
            { Tokens::OP_MINUS_MINUS, OperatorType::Regular },
            { Tokens::BRACKET_ROUND_OPEN, OperatorType::Function },
            { Tokens::BRACKET_SQUARE_OPEN, OperatorType::SliceSubscript },
            { Tokens::OP_DOT, OperatorType::Regular },
            { Tokens::OP_ARROW, OperatorType::Regular },
            { Tokens::OP_PTR, OperatorType::Regular },    // Pointer dereference
            { Tokens::OP_AMPERSAND, OperatorType::Regular },   // Address of
        }
    },
    {
        // 3
        .kind = OperatorPriority::OpKind::Prefix,
        .operators = {
            { Tokens::OP_PLUS_PLUS, OperatorType::Regular },
            { Tokens::OP_MINUS_MINUS, OperatorType::Regular },
            { Tokens::OP_PLUS, OperatorType::Regular },        // Unary plus
            { Tokens::OP_MINUS, OperatorType::Regular },       // Unary minus
            { Tokens::OP_BIT_NOT, OperatorType::Regular },
            { Tokens::OP_LOGIC_NOT, OperatorType::Regular },
            { Tokens::RESERVED_EXPECT, OperatorType::Cast },
        }
    },
    {
        // 5
        .kind = OperatorPriority::OpKind::Infix,
        .operators = {
            { Tokens::OP_MULTIPLY, OperatorType::Regular },
            { Tokens::OP_DIVIDE, OperatorType::Regular },
            { Tokens::OP_MODULOUS, OperatorType::Regular },
            { Tokens::OP_AMPERSAND, OperatorType::Regular },   // Bitwise AND
        }
    },
    {
        // 6
        .kind = OperatorPriority::OpKind::Infix,
        .operators = {
            { Tokens::OP_PLUS, OperatorType::Regular },
            { Tokens::OP_MINUS, OperatorType::Regular },
            { Tokens::OP_BIT_OR, OperatorType::Regular },
            { Tokens::OP_BIT_XOR, OperatorType::Regular },
        }
    },
    {
        // 7
        .kind = OperatorPriority::OpKind::Infix,
        .operators = {
            { Tokens::OP_SHIFT_LEFT, OperatorType::Regular },
            { Tokens::OP_SHIFT_RIGHT, OperatorType::Regular },
        }
    },
    {
        // 8
        .kind = OperatorPriority::OpKind::Infix,
        .operators = {
            { Tokens::OP_LESS_THAN, OperatorType::Regular },
            { Tokens::OP_LESS_THAN_EQ, OperatorType::Regular },
            { Tokens::OP_GREATER_THAN, OperatorType::Regular },
            { Tokens::OP_GREATER_THAN_EQ, OperatorType::Regular },
        }
    },
    {
        // 9
        .kind = OperatorPriority::OpKind::Infix,
        .operators = {
            { Tokens::OP_EQUALS, OperatorType::Regular },
            { Tokens::OP_NOT_EQUALS, OperatorType::Regular },
        }
    },
    {
        // 10
        .kind = OperatorPriority::OpKind::Infix,
        .operators = {
            { Tokens::OP_LOGIC_AND, OperatorType::Regular },
        }
    },
    {
        // 11
        .kind = OperatorPriority::OpKind::Infix,
        .operators = {
            { Tokens::OP_LOGIC_OR, OperatorType::Regular },
        }
    },
    {
        // 12
        .kind = OperatorPriority::OpKind::InfixRight2Left,
        .operators = {
            { Tokens::OP_ASSIGN, OperatorType::Regular },
            { Tokens::OP_ASSIGN_PLUS, OperatorType::Regular },
            { Tokens::OP_ASSIGN_MINUS, OperatorType::Regular },
            { Tokens::OP_ASSIGN_MULTIPLY, OperatorType::Regular },
            { Tokens::OP_ASSIGN_DIVIDE, OperatorType::Regular },
            { Tokens::OP_ASSIGN_MODULOUS, OperatorType::Regular },
            { Tokens::OP_ASSIGN_LEFT_SHIFT, OperatorType::Regular },
            { Tokens::OP_ASSIGN_RIGHT_SHIFT, OperatorType::Regular },
            { Tokens::OP_ASSIGN_BIT_AND, OperatorType::Regular },
            { Tokens::OP_ASSIGN_BIT_OR, OperatorType::Regular },
            { Tokens::OP_ASSIGN_BIT_XOR, OperatorType::Regular },
        }
    },
};

} // Namespace Operators
