#ifndef TYPED_H
#define TYPED_H

#include <cstddef>
#include <type_traits>

template <typename Type, Type initValue, size_t module, size_t id>
class Typed {
    typename std::enable_if< std::is_integral<Type>::value, Type>::type val = initValue;

public:
    typedef Type UnderlyingType;

    Typed() = default;
    // Default copying okay

    explicit constexpr Typed(Type _val) : val(_val) {
    }

    Typed get() const {
        return val;
    }

    bool operator==(Type rhs) const {
        return val==rhs.val;
    }
    bool operator!=(Type rhs) const {
        return val!=rhs.val;
    }

    template <Type startValue = initValue>
    class Allocator {
        Type index = startValue;

    public:
        Type alloc() {
            return Type(index++);
        }
    };
};

#endif // TYPED_H
