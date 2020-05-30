/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef SLICE_H
#define SLICE_H

#include "defines.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <vector>

template <class T>
class Slice {
    T *ptr = nullptr;
    size_t len = 0;

public:
    // No ownership, default copy ctr is fine
    constexpr Slice(const Slice &rhs) = default;
    constexpr Slice &operator=(const Slice &rhs) = default;
    constexpr Slice() = default;

    constexpr Slice(T *ptr, size_t length) : ptr(ptr), len(length) {
    }

    template<
        typename V,
        std::enable_if_t<
            std::is_same_v<std::remove_const_t<T>,V> && std::is_const_v<T>,
            int
        > = 0
    >
    /* implicit conversion */ Slice(const Slice< V > &that) :
        ptr( that.get() ),
        len( that.size() )
    {
    }

    template<typename V, std::enable_if_t<std::is_same_v< std::remove_cv_t<T>, std::remove_cv_t<V> >, int> = 0>
    /* implicit conversion */ Slice(std::vector<V> &vector) :
        ptr(vector.data()), len(vector.size())
    {
    }

    /* implicit */ Slice( std::initializer_list<T> list ) :
        ptr(list.begin()), len(list.size())
    {}

    template<size_t Size, typename V, std::enable_if_t<std::is_same_v< std::remove_cv_t<V>, std::remove_cv_t<T>>, int> = 0>
    /* implicit */ Slice( const std::array<V, Size> &array ) :
        ptr(&array[0]), len(array.size())
    {}

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

    void copy( const Slice source ) {
        for( size_t i=0; i<size(); ++i ) {
            (*this)[i] = source[i];
        }
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

    T* end() {
        return get() + size();
    }

    const T* begin() const {
        return get();
    }

    const T* end() const {
        return get() + size();
    }

    explicit operator bool() const noexcept {
        return size()!=0;
    }
};

template <>
class Slice<const void> {
    const void *ptr = nullptr;
    size_t len = 0;

public:
    // No ownership, default copy ctr is fine
    Slice(const Slice &rhs) = default;
    Slice &operator=(const Slice &rhs) = default;
    Slice() = default;

    constexpr Slice(const void *ptr, size_t length) : ptr(ptr), len(length) {
    }

    Slice( const Slice<void> &rhs );

    /* implicit conversion */
    template<typename T>
    Slice( const Slice<T> &rhs) : ptr(rhs.get()), len(rhs.size()*sizeof(T)) {
    }

    // Accessors
    size_t size() const {
        return len;
    }

    const void *get() const {
        return ptr;
    }
};

template <>
class Slice<void> {
    void *ptr = nullptr;
    size_t len = 0;

public:
    // No ownership, default copy ctr is fine
    Slice(const Slice &rhs) = default;
    Slice &operator=(const Slice &rhs) = default;
    Slice() = default;

    constexpr Slice(void *ptr, size_t length) : ptr(ptr), len(length) {
    }

    /* implicit conversion */
    template<typename T>
    Slice( std::enable_if_t< std::negation_v< std::is_const<T> >, Slice<T> > &rhs) : ptr(rhs.get()), len(rhs.size()*sizeof(T)) {
    }

    // Accessors
    size_t size() const {
        return len;
    }

    void *get() {
        return ptr;
    }

    const void *get() const {
        return ptr;
    }
};

// Slice<const void> constructor can only be fully defined after definition of Slice<void>
inline Slice<const void>::Slice( const Slice<void> &rhs ) : ptr(rhs.get()), len(rhs.size()) {
}

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
    /* implicit */ String( const char *string ) : Slice( string, strlen(string) ) {}
    /* implicit */ String( const std::string &string ) : Slice( string.c_str(), string.size() ) {}
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
            hash< Slice<const char> > realHasher{};

            return realHasher( string );
        }
    };
}

#endif // SLICE_H
