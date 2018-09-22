#ifndef SLICE_H
#define SLICE_H

#include <cstddef>
#include <cassert>

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

        return Slice(ptr+start, end - start - 1);
    }

    Slice<const T> subslice(size_t start, size_t end) const {
        if( end<=start )
            return Slice<const T>();

        assert(start<len);
        assert(end<=len);

        return Slice<const T>(ptr+start, end - start - 1);
    }
};

#endif // SLICE_H
