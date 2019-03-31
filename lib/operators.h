/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef OPERATORS_H
#define OPERATORS_H

#include <unordered_map>
#include <vector>

#include "slice.h"
#include "tokenizer.h"

namespace Operators {

enum class OperatorType { Regular, Function, SliceSubscript };

struct OperatorPriority {
    enum class OpKind { Prefix, Infix, InfixRight2Left, Postfix } kind;

    using OperatorsMap = std::unordered_map< Tokenizer::Tokens, OperatorType >;
    OperatorsMap operators;
};

extern const std::vector< OperatorPriority > operators;

} // Namespace Operators

#endif // OPERATORS_H
