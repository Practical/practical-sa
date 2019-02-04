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

public:
    typedef Type UnderlyingType;
    static constexpr Type InitValue = initValue;

    Typed() = default;
    // Default copying okay

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
};

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
    return out << "Typed(" << t.get() << ")";
}

#endif // TYPED_H
