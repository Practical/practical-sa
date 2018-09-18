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

    const Slice subslice(size_t start) const {
        return subslice(start, len);
    }

    Slice<int> subslice(size_t start, size_t end) {
        // It is legal to cast away constness if you know the underlying object is mutable
        return ( static_cast<const Slice *>(this)->subslice(start, end) );
        //return const_cast<Slice>( static_cast<const Slice *>(this)->subslice(start, end) );
    }

    const Slice subslice(size_t start, size_t end) const {
        assert(start<len);
        assert(end<=len);
        return Slice(ptr+start, len - start - end);
    }
};

#endif // SLICE_H
