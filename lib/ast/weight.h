/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_WEIGHT_H
#define AST_WEIGHT_H

namespace AST {

struct Weight {
    unsigned weight = 0, length = 0;

    Weight() = default;

    constexpr explicit Weight( unsigned _weight, unsigned _length = 1 ) :
        weight(_weight), length(_length)
    {}

    constexpr bool operator<( const Weight &that ) const noexcept {
        return
                length<that.length ||
                (length==that.length && weight<that.weight);
    }

    constexpr bool operator<=( const Weight &that ) const noexcept {
        return
                length<that.length ||
                (length==that.length && weight<=that.weight);
    }

    constexpr bool operator==( const Weight &that ) const noexcept {
        return weight==that.weight && length==that.length;
    }

    constexpr bool operator!=( const Weight &that ) const noexcept {
        return !( (*this)==that );
    }

    constexpr bool operator>( const Weight &that ) const noexcept {
        return !( (*this)<=that );
    }

    constexpr bool operator>=( const Weight &that ) const noexcept {
        return !( (*this)<that );
    }

    constexpr Weight &operator+=( const Weight &that ) noexcept {
        weight += that.weight;
        length += that.length;

        return *this;
    }

    constexpr Weight operator+( const Weight &that ) const noexcept {
        Weight result(*this);
        return result+=that;
    }

    constexpr Weight &operator-=( const Weight &that ) noexcept {
        weight -= that.weight;
        length -= that.length;

        return *this;
    }

    constexpr Weight operator-( const Weight &that ) const noexcept {
        Weight result(*this);
        return result-=that;
    }

    static constexpr Weight max() noexcept {
        return Weight(
                std::numeric_limits<unsigned>::max(),
                std::numeric_limits<unsigned>::max() );
    }
};

} // namespace AST

#endif // AST_WEIGHT_H
