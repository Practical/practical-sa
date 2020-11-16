#include "ast/weight.h"

std::ostream &operator<<( std::ostream &out, const AST::Weight &weight ) {
    return out<<"Weight("<<weight.weight<<","<<weight.length<<")";
}

