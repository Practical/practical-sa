#ifndef DEFINES_H
#define DEFINES_H

#include <cmath>
#include <cstdio>
#include <cstring>
#include <exception>
#include <memory>
#include <utility>

class compile_error : public std::exception {
    size_t line, col;
    std::unique_ptr<char> msg;
public:
    compile_error(const char *msg, size_t line, size_t col) : line(line), col(col) {
        size_t buffsize = strlen(msg) + 100;
        this->msg = std::unique_ptr<char>(new char[buffsize]);
        snprintf(this->msg.get(), buffsize, "%s at %lu:%lu", msg, line, col);
    }

    const char *what() const noexcept {
        return msg.get();
    }

    size_t getLine() const {
        return line;
    }

    size_t getCol() const {
        return col;
    }
};

template <typename T, typename... ARGS>
std::unique_ptr<T> safenew(ARGS&&... args) {
    return std::unique_ptr<T>( new T(std::forward<ARGS...>(args...)) );
}

// std::forward does not handle the empty case well
template <typename T>
std::unique_ptr<T> safenew() {
    return std::unique_ptr<T>( new T() );
}

static constexpr double GoldenRatio = (1 + sqrt(5.0)) / 2;

#endif // DEFINES_H
