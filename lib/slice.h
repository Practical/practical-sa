#ifndef SLICE_H
#define SLICE_H

#include <cstddef>
#include <cassert>
#include <iostream>

template <class T>
class Slice {
    T *ptr = nullptr;
    size_t len = 0;

    typedef typename std::conditional<
            std::is_const<T>::value,
            const std::vector<
                    typename std::remove_const<T>::type>,
            std::vector<T>
        >::type vector_type;
public:
    // No ownership, default copy ctr is fine
    Slice(const Slice &rhs) = default;
    Slice &operator=(const Slice &rhs) = default;
    Slice() = default;

    Slice(T *ptr, size_t length) : ptr(ptr), len(length) {
    }

    /* implicit conversion */ Slice(vector_type &vector) :
        ptr(vector.data()), len(vector.size())
    {
    }

    // Accessors
    size_t size() const {
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
    for( size_t i=0; i<data.size(); ++i ) {
        out<<data[i];
    }

    return out;
}

static inline std::string sliceToString( const Slice<const char> &src ) {
    if( src.size()==0 )
        return std::string();

    return std::string(&src[0], src.size());
}

#endif // SLICE_H
