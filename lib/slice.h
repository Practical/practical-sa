#ifndef SLICE_H
#define SLICE_H

#include <cstddef>
#include <cassert>
#include <iostream>

template <class T>
class Slice {
    T *ptr;
    size_t len;

public:
    // No ownership, default copy ctr is fine
    Slice(const Slice &rhs) = default;
    Slice &operator=(const Slice &rhs) = default;
    Slice() = default;

    Slice(T *ptr, size_t length) : ptr(ptr), len(length) {}

    // Accessors
    size_t length() const {
        return len;
    }

    T *get() {
        return ptr;
    }

    const T *get() const {
        return ptr;
    }

    T &operator[](size_t index) {
        assert(index<len);
        return ptr[index];
    }

    const T &operator[](size_t index) const {
        assert(index<len);
        return ptr[index];
    }

    Slice subslice(size_t start) {
        return subslice(start, len);
    }

    Slice<const T> subslice(size_t start) const {
        return subslice(start, len);
    }

    Slice subslice(size_t start, size_t end) {
        if( end<=start )
            return Slice();

        assert(start<len);
        assert(end<=len);

        return Slice(ptr+start, end - start);
    }

    Slice<const T> subslice(size_t start, size_t end) const {
        if( end<=start )
            return Slice<const T>();

        assert(start<len);
        assert(end<=len);

        return Slice<const T>(ptr+start, end - start);
    }
};

template <typename T>
std::ostream &operator<<( std::ostream &out, const Slice<T> &data ) {
    for( size_t i=0; i<data.length(); ++i ) {
        out<<data[i];
    }

    return out;
}

#endif // SLICE_H
