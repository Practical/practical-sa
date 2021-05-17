/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2021 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_HASH_MODIFIERS_H
#define AST_HASH_MODIFIERS_H

namespace AST {
    static constexpr size_t PointerModifier = 4;
    static constexpr size_t ReferenceModifier = 6;
    static constexpr size_t MutableModifier = 10;
    static constexpr size_t ArrayModifier = 14;
    static constexpr size_t StructModifier = 22;

    static constexpr size_t RecursiveStructHash = 0x2bcdadbd0483cb7c;
} // namespace AST

#endif // AST_HASH_MODIFIERS_H
