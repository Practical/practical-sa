/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef TYPED_H
#define TYPED_H

#include <cstddef>
#include <functional>
#include <iostream>
#include <type_traits>

template <typename Type, Type initValue, size_t module, size_t id>
class Typed {
    static_assert( std::is_integral<Type>::value, "Typed only works with integral types" );
    Type val = initValue;
    static const char * const typeName;

public:
    typedef Type UnderlyingType;
    static constexpr Type InitValue = initValue;

    Typed() = default;
    Typed(const Typed &that) = default;
    Typed &operator=(const Typed &that) = default;

    explicit constexpr Typed(Type _val) : val(_val) {
    }

    Type get() const {
        return val;
    }

    bool operator==(Typed rhs) const {
        return val==rhs.val;
    }
    bool operator!=(Typed rhs) const {
        return val!=rhs.val;
    }

    template <Type startValue = initValue>
    class Allocator {
        Type index = startValue;

    public:
        Typed allocate() {
            return Typed(++index);
        }
    };

    static const char *getName() {
        return typeName;
    }
};

// If declaring the type in the global namespace, use these two macros:
// In h file:
#define DECL_TYPED( name, T, init, moduleId ) \
        using name = Typed< T, init, moduleId, __LINE__ >; \
        template <> const char * const name::typeName
// In cpp file:
#define DEF_TYPED( name ) template<> const char * const name::typeName = #name

// If declaring the type to be a member of the namespace:
// In h file, outside namespace:
#define DECL_TYPED_NS( name_space, name, T, init, moduleId ) \
        namespace name_space { \
            using name = Typed< T, init, moduleId, __LINE__ >; \
        } \
        template <> const char * const name_space::name::typeName

// In cpp file, outside namespace:
#define DEF_TYPED_NS( name_space, name ) template<> const char * const name_space::name::typeName = #name

namespace std {
    template <typename Type, Type initValue, size_t module, size_t id>
    struct hash< Typed<Type, initValue, module, id> > {
    private:
        std::hash<Type> hasher;

    public:
        constexpr size_t operator()(const Typed<Type, initValue, module, id> &val) const {
            return hasher(val.get());
        }
    };
}

template<typename T, T init, size_t mod, size_t id>
std::ostream &operator<<(std::ostream &out, Typed<T, init, mod, id> t) {
    return out << Typed<T, init, mod, id>::getName() << "(" << t.get() << ")";
}

// F$(&!@# C++ can't tell uint8_t from char, calls wrong operator<<
template<uint8_t init, size_t mod, size_t id>
std::ostream &operator<<(std::ostream &out, Typed<uint8_t, init, mod, id> t) {
    return out << Typed<uint8_t, init, mod, id>::getName() << "(" << (int)t.get() << ")";
}

template<int8_t init, size_t mod, size_t id>
std::ostream &operator<<(std::ostream &out, Typed<int8_t, init, mod, id> t) {
    return out << Typed<int8_t, init, mod, id>::getName() << "(" << (int)t.get() << ")";
}

#endif // TYPED_H
