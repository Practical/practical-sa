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
