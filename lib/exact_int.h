#ifndef EXACT_INT_H
#define EXACT_INT_H

#include "asserts.h"

#include <cstdint>
#include <iostream>
#include <variant>

// An integer type that can hold the whole range of 128 bits, both signed and unsigned
class ExactInt {
public:
    enum class Type { SIGNED, UNSIGNED };

private:
    typedef signed __int128 Ts;
    typedef unsigned __int128 Tu;

    union {
        Ts s = 0;
        Tu u;
    };

    Type type = Type::SIGNED;

    // Prevent accidental conversion from bool - private declaration with no definition
    explicit ExactInt(bool value);

public:
    // Default copying okay

    /* implicit */ ExactInt(Ts value) : s(value), type(Type::SIGNED) {}
    /* implicit */ ExactInt(Tu value) : u(value), type(Type::UNSIGNED) {}

    /* implicit */ ExactInt(int8_t value) : ExactInt( (Ts)value ) {}
    /* implicit */ ExactInt(int16_t value) : ExactInt( (Ts)value ) {}
    /* implicit */ ExactInt(int32_t value) : ExactInt( (Ts)value ) {}
    /* implicit */ ExactInt(int64_t value) : ExactInt( (Ts)value ) {}

    /* implicit */ ExactInt(uint8_t value) : ExactInt( (Tu)value ) {}
    /* implicit */ ExactInt(uint16_t value) : ExactInt( (Tu)value ) {}
    /* implicit */ ExactInt(uint32_t value) : ExactInt( (Tu)value ) {}
    /* implicit */ ExactInt(uint64_t value) : ExactInt( (Tu)value ) {}

    // Comparison operators
    bool operator<(const ExactInt &that) const {
        if( nonNegative() ) {
            if( !that.nonNegative() )
                return false;

            return u<that.u;
        } else {
            if( that.nonNegative() )
                return true;

            return s<that.s;
        }
    }

    bool operator==(const ExactInt &that) const {
        return nonNegative()==that.nonNegative() && u==that.u;
    }

    bool operator<=(const ExactInt &that) const {
        return *this<that || *this==that;
    }
    bool operator>(const ExactInt &that) const {
        return that<*this;
    }
    bool operator>=(const ExactInt &that) const {
        return that<=*this;
    }
    bool operator!=(const ExactInt &that) const {
        return ! (*this==that);
    }

    Tu getUnsigned() const {
        ASSERT( nonNegative() );
        return u;
    }

    Ts getSigned() const {
        ASSERT( type==Type::SIGNED || s>=0 );
        return s;
    }

    Type getType() const {
        return type;
    }
private:
    bool nonNegative() const {
        return type==Type::UNSIGNED || s>=0;
    }
};

std::ostream &operator<<(std::ostream &out, unsigned __int128 val) {
    char output[50];
    int i=0;

    while( val!=0 ) {
        output[i++] = (val % 10) + '0';
        val /= 10;
    }

    if( i==0 )
        output[i++] = '0';

    while( i!=0 ) {
        out<<output[--i];
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, signed __int128 val) {
    if( val<0 ) {
        val = -val;
        out<<'-';
    }

    return out<<static_cast<unsigned __int128>(val);
}

std::ostream &operator<<(std::ostream &out, ExactInt i) {
    switch( i.getType() ) {
    case ExactInt::Type::SIGNED:
        out<<i.getSigned();
        break;
    case ExactInt::Type::UNSIGNED:
        out<<i.getUnsigned();
        break;
    }

    return out;
}

#endif // EXACT_INT_H
