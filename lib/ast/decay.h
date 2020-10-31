/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_DECAY_H
#define AST_DECAY_H

#include "ast/cast_chain.h"

namespace AST {

void decayInit();

std::vector< StaticTypeImpl::CPtr > decay(
        std::unordered_map< StaticTypeImpl::CPtr, CastChain::Junction > &paths,
        StaticTypeImpl::CPtr type, unsigned length, unsigned weight );

}

#endif // AST_DECAY_H
