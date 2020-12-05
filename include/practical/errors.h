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

#include <practical/practical.h>

namespace Tokenizer {
    struct Token;
}

namespace PracticalSemanticAnalyzer {

class compile_error : public std::exception {
    SourceLocation location;
    std::unique_ptr<char[]> msg;
public:
    compile_error(const char *msg, const SourceLocation &location) : location(location) {
        setMsg(msg);
    }

    compile_error(const SourceLocation &location) : location(location) {}

    const char *what() const noexcept {
        assert(msg);
        return msg.get();
    }

    const SourceLocation getLocation() const {
        return location;
    }

protected:
    void setMsg(const char *msg) {
        size_t buffsize = strlen(msg) + 100;
        this->msg = std::unique_ptr<char[]>(new char[buffsize]);
        snprintf(this->msg.get(), buffsize, "%s at %u:%u", msg, location.line, location.col);
    }
};

class tokenizer_error : public compile_error {
public:
    tokenizer_error(const char *msg, const SourceLocation &location) : compile_error(msg, location) {
    }
};

class parser_error : public compile_error {
public:
    parser_error(const char *msg, const SourceLocation &location) : compile_error(msg, location) {
    }
};

class pass1_error : public compile_error {
public:
    pass1_error(const char *msg, const SourceLocation &location) : compile_error(msg, location) {
    }
};

class pass2_error : public compile_error {
public:
    pass2_error(const char *msg, const SourceLocation &location) : compile_error(msg, location) {
    }
};

class IllegalLiteral : public compile_error {
public:
    IllegalLiteral(const char *msg, const SourceLocation &location) : compile_error(msg, location) {
    }
};

class InvalidEscapeSequence : public IllegalLiteral {
public:
    InvalidEscapeSequence(const SourceLocation &location) :
        IllegalLiteral("Invalid escape sequence in string literal", location)
    {}
};

class UndefinedBehavior : public compile_error {
public:
    UndefinedBehavior(const char *msg, const SourceLocation &location );
};

class IncompatibleTypes : public compile_error {
public:
    IncompatibleTypes(StaticType::CPtr left, StaticType::CPtr right, const SourceLocation &location);
};

class SymbolRedefined : public compile_error {
public:
    SymbolRedefined(String symbol, const SourceLocation &location);
};

class SymbolNotFound : public compile_error {
public:
    SymbolNotFound(String symbol, const SourceLocation &location);
};

class CannotTakeValueOfFunction : public compile_error {
public:
    CannotTakeValueOfFunction(const Tokenizer::Token *identifier);
};

class TryToCallNonCallable : public compile_error {
public:
    TryToCallNonCallable(const Tokenizer::Token *identifier);
};

class NoMatchingOverload : public compile_error {
public:
    NoMatchingOverload(const Tokenizer::Token *identifier);
};

class AmbiguousOverloads : public compile_error {
public:
    AmbiguousOverloads(const Tokenizer::Token *identifier);
};

class CastError : public compile_error {
public:
    CastError(
            const char *msg, StaticType::CPtr src, StaticType::CPtr dst, bool implicit,
            const SourceLocation &location);
};

class CastNotAllowed : public CastError {
public:
    CastNotAllowed(StaticType::CPtr src, StaticType::CPtr dst, bool implicit, const SourceLocation &location);
};

class AmbiguousCast : public CastError {
public:
    AmbiguousCast(StaticType::CPtr src, StaticType::CPtr dst, bool implicit, const SourceLocation &location);
};

class LValueRequired : public compile_error {
public:
    LValueRequired(StaticType::CPtr wrongType, const SourceLocation &location);
};

class PointerExpected : public compile_error {
public:
    PointerExpected(StaticType::CPtr wrongType, const SourceLocation &location);
};

class KnownRuntimeViolation : public compile_error {
public:
    KnownRuntimeViolation(const char *message, const SourceLocation &location);
};

class UnidentifiedAbiString : public compile_error {
public:
    UnidentifiedAbiString( const SourceLocation &location ) :
        compile_error( "Unidentified ABI string", location )
    {}
};

class MultipleDefinitions : public compile_error {
public:
    MultipleDefinitions( const SourceLocation &location ) :
        compile_error( "Symbol defined multiple times", location )
    {}
};

} // PracticalSemanticAnalyzer

#endif // PRACTICAL_ERRORS_H
