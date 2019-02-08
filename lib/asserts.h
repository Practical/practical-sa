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

#define ASSERT(cond) if( !(cond) ) (Asserts::LoggerCleanup(std::cerr), std::cerr)<<"Compiler assert failure for " #cond " at " __FILE__ ":" << __LINE__ << " "
#define ABORT() (Asserts::LoggerCleanup(std::cerr), std::cerr)<<"Compiler aborted at " __FILE__ ":" << __LINE__ << " "

#endif // ASSERTS_H
