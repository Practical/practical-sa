/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef SCOPE_TRACING_H
#define SCOPE_TRACING_H

#include "asserts.h"
#include "nocopy.h"

#include <cassert>
#include <exception>
#include <iostream>
#include <string>

class ScopeTrace : NoCopy {
    std::string name;
    unsigned int myNesting;
    static thread_local unsigned int nesting;

public:
    ScopeTrace(const char *name) : name(name), myNesting(nesting++) {
        indent(std::cerr) << "Entering " << name << "\n";
    }

    ~ScopeTrace() {
        --nesting;
        ASSERT(nesting==myNesting);
        auto ex = std::current_exception();
        if( ex )
            indent(std::cerr) << "Leaving " << name << "\n";
        else
            indent(std::cerr) << "Throwing out of " << name << "\n";
    }

private:
    std::ostream &indent(std::ostream &out) {
        for( unsigned i = 0; i<myNesting; ++i )
            out << ' ';

        return out;
    }
};

#endif // SCOPE_TRACING_H
