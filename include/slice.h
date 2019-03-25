#ifndef SLICE_H
#define SLICE_H

#include "defines.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

template <class T>
class Slice {
    T *ptr = nullptr;
    size_t len = 0;

    using vector_type = std::conditional_t<
            std::is_const<T>::value,
            const std::vector< std::remove_const_t<T> >,
            std::vector<T>
        >;
public:
    // No ownership, default copy ctr is fine
    Slice(const Slice &rhs) = default;
    Slice &operator=(const Slice &rhs) = default;
    Slice() = default;

    constexpr Slice(T *ptr, size_t length) : ptr(ptr), len(length) {
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

    // Compare the data pointed by the two slices. Allows using slice as key in hash
    bool operator==(const Slice &rhs) const {
        if( size() != rhs.size() )
            return false;

        for( size_t i=0; i<size(); ++i ) {
            if( (*this)[i]!=rhs[i] )
                return false;
        }

        return true;
    }

    T* begin() {
        return get();
    }

    const T* begin() const {
        return get();
    }

    const T* end() const {
        return get() + size();
    }
};

template <typename T>
std::ostream &operator<<( std::ostream &out, const Slice<T> &data ) {
    for( size_t i=0; i<data.size(); ++i ) {
        out<<data[i];
    }

    return out;
}

class String : public Slice<const char> {
public:
    using Slice::Slice;

    /* implicit */ String( Slice<const char> slice ) : Slice(slice) {}
    String( const char *string ) : Slice( string, strlen(string) ) {}
};

static inline std::string sliceToString( const String &src ) {
    if( src.size()==0 )
        return std::string();

    return std::string(&src[0], src.size());
}

static inline String toSlice(const char *str) {
    return String(str, strlen(str));
}

namespace std {
    template <typename T>
    struct hash< Slice<T> > {
        static constexpr size_t FibonacciHashMultiplier = static_cast<size_t>(-1) / GoldenRatio;

        constexpr size_t operator()(const Slice<T> &slice) const {
            size_t result = 0;

            std::hash< typename std::remove_cv<T>::type > recursive;
            for( size_t i=0; i<slice.size(); ++i ) {
                result += recursive(slice[i]);
                result *= FibonacciHashMultiplier;
            }

            return result;
        }
    };

    template <>
    struct hash< String > {
        constexpr size_t operator()( const String &string ) const {
            hash< Slice<const char> > realHasher;

            return realHasher( string );
        }
    };
}

#endif // SLICE_H
