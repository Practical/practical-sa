/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef ASSERTS_H
#define ASSERTS_H

#include <stdlib.h>
#include <iostream>

namespace Asserts {

struct LoggerCleanup {
    std::ostream &logStream;

public:
    explicit LoggerCleanup(std::ostream &out) : logStream(out) {}
    ~LoggerCleanup() __attribute__((noreturn)) { logStream<<std::endl; abort(); }
};

}

#define ASSERT(cond) if( !(cond) ) (Asserts::LoggerCleanup(std::cerr), std::cerr)<<"ASSERT failure for " #cond " at " __FILE__ ":" << __LINE__ << " "
#define ABORT() (Asserts::LoggerCleanup(std::cerr), std::cerr)<<"ABORT at " __FILE__ ":" << __LINE__ << " "

#endif // ASSERTS_H
