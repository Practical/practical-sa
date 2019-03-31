/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef DEFINES_H
#define DEFINES_H

#include <assert.h>
#include <cstdio>
#include <cstring>
#include <exception>
#include <memory>
#include <utility>

template <typename T, typename... ARGS>
std::unique_ptr<T> safenew(ARGS&&... args) {
    return std::unique_ptr<T>( new T(std::forward<ARGS...>(args...)) );
}

// std::forward does not handle the empty case well
template <typename T>
std::unique_ptr<T> safenew() {
    return std::unique_ptr<T>( new T() );
}

// static constexpr double GoldenRatio = (1 + sqrt(5.0)) / 2;
static constexpr double GoldenRatio = 1.618033988749895;

#endif // DEFINES_H
