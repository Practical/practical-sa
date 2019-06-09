/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef FULL_RANGE_INT_H
#define FULL_RANGE_INT_H

#include "practical-sa.h"

#include <variant>

class FullRangeInt {
    std::variant<LongEnoughInt, LongEnoughIntSigned> value;

public:
    /* implicit */ FullRangeInt( LongEnoughInt value = 0 );
    /* implicit */ FullRangeInt( LongEnoughIntSigned value );

    /* implicit */ FullRangeInt( uint8_t value ) : FullRangeInt( static_cast<LongEnoughInt>(value) ) {}
    /* implicit */ FullRangeInt( uint16_t value ) : FullRangeInt( static_cast<LongEnoughInt>(value) ) {}
    /* implicit */ FullRangeInt( uint32_t value ) : FullRangeInt( static_cast<LongEnoughInt>(value) ) {}

    /* implicit */ FullRangeInt( int8_t value ) : FullRangeInt( static_cast<LongEnoughIntSigned>(value) ) {}
    /* implicit */ FullRangeInt( int16_t value ) : FullRangeInt( static_cast<LongEnoughIntSigned>(value) ) {}
    /* implicit */ FullRangeInt( int32_t value ) : FullRangeInt( static_cast<LongEnoughIntSigned>(value) ) {}

    // Default copy are okay for us
    
    // Define the whole nine yards of operators
    FullRangeInt &operator+=( const FullRangeInt &rhs );
    FullRangeInt &operator-=( const FullRangeInt &rhs );
    FullRangeInt &operator*=( const FullRangeInt &rhs );
    FullRangeInt &operator/=( const FullRangeInt &rhs );
    FullRangeInt &operator%=( const FullRangeInt &rhs );

    FullRangeInt &operator&=( const FullRangeInt &rhs );
    FullRangeInt &operator|=( const FullRangeInt &rhs );
    FullRangeInt &operator^=( const FullRangeInt &rhs );

    // Oh how I wish for C++20's operator<=>
    friend bool operator<( const FullRangeInt &lhs, const FullRangeInt &rhs );
    friend bool operator<=( const FullRangeInt &lhs, const FullRangeInt &rhs );
    friend bool operator==( const FullRangeInt &lhs, const FullRangeInt &rhs );
    friend bool operator!=( const FullRangeInt &lhs, const FullRangeInt &rhs );
    friend bool operator>( const FullRangeInt &lhs, const FullRangeInt &rhs );
    friend bool operator>=( const FullRangeInt &lhs, const FullRangeInt &rhs );

    friend std::ostream &operator<<(std::ostream &out, FullRangeInt i);
private:
    void normalize();
    bool isNormalized() const;
};

inline FullRangeInt operator+( FullRangeInt rhs, FullRangeInt lhs ) {
    return rhs+=lhs;
}

inline FullRangeInt operator+( FullRangeInt value ) {
    return value;
}

inline FullRangeInt operator-( FullRangeInt rhs, FullRangeInt lhs ) {
    return rhs-=lhs;
}

inline FullRangeInt operator-( FullRangeInt value ) {
    return 0-value;
}

inline FullRangeInt operator*( FullRangeInt rhs, FullRangeInt lhs ) {
    return rhs*=lhs;
}

inline FullRangeInt operator/( FullRangeInt rhs, FullRangeInt lhs ) {
    return rhs/=lhs;
}

inline FullRangeInt operator%( FullRangeInt rhs, FullRangeInt lhs ) {
    return rhs%=lhs;
}

inline FullRangeInt operator&( FullRangeInt rhs, FullRangeInt lhs ) {
    return rhs&=lhs;
}

inline FullRangeInt operator|( FullRangeInt rhs, FullRangeInt lhs ) {
    return rhs|=lhs;
}

inline FullRangeInt operator^( FullRangeInt rhs, FullRangeInt lhs ) {
    return rhs^=lhs;
}

#endif // FULL_RANGE_INT_H
