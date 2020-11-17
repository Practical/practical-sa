/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PRACTICAL_DEFINES_H
#define PRACTICAL_DEFINES_H

#include <assert.h>
#include <cstdio>
#include <cstring>
#include <exception>
#include <memory>
#include <utility>

template<typename T, typename... Args>
std::unique_ptr<T> safenew(Args&&... args) {
    return std::unique_ptr<T>( new T(std::forward<Args>(args)...) );
}

static constexpr size_t FibonacciHashMultiplier = 0x9e3779b97f4a7c15; //static_cast<size_t>(-1) / GoldenRatio;

#endif // PRACTICAL_DEFINES_H
