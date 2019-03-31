/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PRACTICAL_ERRORS_H
#define PRACTICAL_ERRORS_H

#include <practical-sa.h>

namespace Tokenizer {
    struct Token;
}

namespace PracticalSemanticAnalyzer {

class compile_error : public std::exception {
    size_t line, col;
    std::unique_ptr<char> msg;
public:
    compile_error(const char *msg, size_t line, size_t col) : line(line), col(col) {
        setMsg(msg);
    }

    compile_error(size_t line, size_t col) : line(line), col(col) {}

    const char *what() const noexcept {
        assert(msg);
        return msg.get();
    }

    size_t getLine() const {
        return line;
    }

    size_t getCol() const {
        return col;
    }

protected:
    void setMsg(const char *msg) {
        size_t buffsize = strlen(msg) + 100;
        this->msg = std::unique_ptr<char>(new char[buffsize]);
        snprintf(this->msg.get(), buffsize, "%s at %lu:%lu", msg, line, col);
    }
};

class tokenizer_error : public compile_error {
public:
    tokenizer_error(const char *msg, size_t line, size_t col) : compile_error(msg, line, col) {
    }
};

class parser_error : public compile_error {
public:
    parser_error(const char *msg, size_t line, size_t col) : compile_error(msg, line, col) {
    }
};

class pass1_error : public compile_error {
public:
    pass1_error(const char *msg, size_t line, size_t col) : compile_error(msg, line, col) {
    }
};

class pass2_error : public compile_error {
public:
    pass2_error(const char *msg, size_t line, size_t col) : compile_error(msg, line, col) {
    }
};

class CastNotAllowed : public compile_error {
public:
    CastNotAllowed(const StaticType &src, const StaticType &dst, bool implicit, size_t line, size_t col);
};

class SymbolRedefined : public compile_error {
public:
    SymbolRedefined(String symbol, size_t line, size_t col);
};

class SymbolNotFound : public compile_error {
public:
    SymbolNotFound(String symbol, size_t line, size_t col);
};

class CannotTakeValueOfFunction : public compile_error {
public:
    CannotTakeValueOfFunction(const Tokenizer::Token *identifier);
};

class TryToCallNonCallable : public compile_error {
public:
    TryToCallNonCallable(const Tokenizer::Token *identifier);
};

} // PracticalSemanticAnalyzer

#endif // PRACTICAL_ERRORS_H
