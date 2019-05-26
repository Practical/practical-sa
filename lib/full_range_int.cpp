#include "full_range_int.h"

#include "asserts.h"

#include <iostream>

std::ostream &operator<<( std::ostream &out, const std::variant< LongEnoughInt, LongEnoughIntSigned > &value ) {
    struct Visitor {
        std::ostream &out;

        void operator()( LongEnoughInt val ) {
            out<<"Unsigned("<<val<<")";
        }

        void operator()( LongEnoughIntSigned val ) {
            out<<"Signed("<<val<<")";
        }
    };

    std::visit( Visitor{ .out=out }, value );

    return out;
}

FullRangeInt::FullRangeInt( LongEnoughInt value ) : value(value)
{}

FullRangeInt::FullRangeInt( LongEnoughIntSigned value ) : value(value)
{
    normalize();
}

using SignedUnsignedValue = std::variant<LongEnoughInt, LongEnoughIntSigned>;

template <typename Operations>
static auto binaryOperator( const SignedUnsignedValue &lhs, const SignedUnsignedValue &rhs ) {
    class OuterVisitor {
        const SignedUnsignedValue &rhs;
    public:
        OuterVisitor( const SignedUnsignedValue &rhs ) : rhs(rhs) {}

        auto operator()( LongEnoughInt left ) {
            class InnerVisitor {
                LongEnoughInt left;
            public:
                InnerVisitor( LongEnoughInt left ) : left(left) {}

                auto operator()( LongEnoughInt right ) {
                    return Operations::operation( left, right );
                }

                auto operator()( LongEnoughIntSigned right ) {
                    return Operations::operation( left, right );
                }
            };

            return std::visit( InnerVisitor(left), rhs );
        }

        auto operator()( LongEnoughIntSigned left ) {
            class InnerVisitor {
                LongEnoughIntSigned left;
            public:
                InnerVisitor( LongEnoughIntSigned left ) : left(left) {}

                auto operator()( LongEnoughInt right ) {
                    return Operations::operation( left, right );
                }

                auto operator()( LongEnoughIntSigned right ) {
                    return Operations::operation( left, right );
                }
            };

            return std::visit( InnerVisitor(left), rhs );
        }
    };

    return std::visit( OuterVisitor( rhs ), lhs );
}

FullRangeInt &FullRangeInt::operator+=( const FullRangeInt &rhs ) {
    struct Operations {
        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughInt right ) {
            LongEnoughInt result;
            if( __builtin_add_overflow( left, right, &result ) ) {
                ABORT() << "TODO implement overflow behavior";
            }

            return result;
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            ASSERT( left<0 )<<"Input value not normalized";

            if( right>INTMAX_MAX ) {
                // Positive right is larger than negative left can get, so the result is guaranteed to be positive
                right+=left;

                return right;
            }

            // Value is guaranteed to fit within the signed result
            left+=right;

            // Normalize the result
            if( left<0 )
                return left;

            return static_cast<LongEnoughInt>(left);
        }

        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            // Since addition is commutative, use the above implementation
            return operation( right, left );
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            ASSERT( left<0 && right<0 )<<"Input values to add are not normalized";

            LongEnoughIntSigned result;
            if( __builtin_add_overflow( left, right, &result ) ) {
                ABORT() << "TODO implement overflow behavior";
            }

            return result;
        }
    };

    value = binaryOperator<Operations>( value, rhs.value );
    ASSERT( isNormalized() ) << "FullRangeInt is not normalized at end of addition: "<<value;

    return *this;
}

FullRangeInt &FullRangeInt::operator-=( const FullRangeInt &rhs ) {
    struct Operations {
        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughInt right ) {
            if( right>left ) {
                LongEnoughIntSigned result;
                if( __builtin_sub_overflow( left, right, &result ) ) {
                    ABORT() << "TODO implement overflow behavior";
                }

                return result;
            }

            LongEnoughInt result = left - right;

            return result;
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            ASSERT( left<0 )<<"Input value not normalized";

            LongEnoughIntSigned result;
            if( __builtin_sub_overflow( left, right, &result ) ) {
                ABORT() << "TODO implement overflow behavior";
            }

            return result;
        }

        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            ASSERT( right<0 )<<"Input value not normalized";

            LongEnoughInt result;
            if( __builtin_sub_overflow( left, right, &result ) ) {
                ABORT() << "TODO implement overflow behavior";
            }

            return result;
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            ASSERT( left<0 && right<0 )<<"Input values to add are not normalized";

            if( left > right ) {
                // End result is positive with no chance of overflow
                return static_cast<LongEnoughInt>( left - right );
            }

            LongEnoughIntSigned result;
            if( __builtin_sub_overflow( left, right, &result ) ) {
                ABORT() << "TODO implement overflow behavior";
            }

            return result;
        }
    };

    value = binaryOperator<Operations>( value, rhs.value );
    ASSERT( isNormalized() ) << "FullRangeInt is not normalized at end of subtraction: "<<value;

    return *this;
}

FullRangeInt &FullRangeInt::operator*=( const FullRangeInt &rhs ) {
    struct Operations {
        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughInt right ) {
            LongEnoughInt result;

            if( __builtin_mul_overflow( left, right, &result ) ) {
                ABORT() << "TODO implement overflow behavior";
            }

            return result;
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            LongEnoughIntSigned result;

            if( __builtin_mul_overflow( left, right, &result ) ) {
                ABORT() << "TODO implement overflow behavior";
            }

            return result;
        }

        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            LongEnoughIntSigned result;

            if( __builtin_mul_overflow( left, right, &result ) ) {
                ABORT() << "TODO implement overflow behavior";
            }

            return result;
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            LongEnoughInt result;

            if( __builtin_mul_overflow( left, right, &result ) ) {
                ABORT() << "TODO implement overflow behavior";
            }

            return result;
        }
    };

    value = binaryOperator<Operations>( value, rhs.value );
    ASSERT( isNormalized() ) << "FullRangeInt is not normalized at end of multiplication: "<<value;

    return *this;
}

FullRangeInt &FullRangeInt::operator/=( const FullRangeInt &rhs ) {
    struct Operations {
        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughInt right ) {
            return left / right;
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            return static_cast<LongEnoughIntSigned>( -(-left/right) );
        }

        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            return static_cast<LongEnoughIntSigned>( -(left/-right) );
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            return static_cast<LongEnoughInt>( left/right );
        }
    };

    value = binaryOperator<Operations>( value, rhs.value );
    ASSERT( isNormalized() ) << "FullRangeInt is not normalized at end of division: "<<value;

    return *this;
}

FullRangeInt &FullRangeInt::operator%=( const FullRangeInt &rhs ) {
    struct Operations {
        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughInt right ) {
            return left%right;
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            return - static_cast<LongEnoughIntSigned>( (-left) % right );
        }

        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            return static_cast<LongEnoughInt>( left % -right );
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            return left%right;
        }
    };

    value = binaryOperator<Operations>( value, rhs.value );
    ASSERT( isNormalized() ) << "FullRangeInt is not normalized at end of modulous: "<<value;

    return *this;
}

FullRangeInt &FullRangeInt::operator&=( const FullRangeInt &rhs ) {
    struct Operations {
        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughInt right ) {
            left &= right;

            return left;
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            ABORT()<<"Signed unsigned bitwise &";
        }

        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            ABORT()<<"Signed unsigned bitwise &";
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            left &= right;

            return left;
        }
    };

    value = binaryOperator<Operations>( value, rhs.value );
    ASSERT( isNormalized() ) << "FullRangeInt is not normalized at end of bitwise AND: "<<value;

    return *this;
}

FullRangeInt &FullRangeInt::operator|=( const FullRangeInt &rhs ) {
    struct Operations {
        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughInt right ) {
            left |= right;
            return left;
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            ABORT()<<"Signed unsigned bitwise |";
        }

        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            ABORT()<<"Signed unsigned bitwise |";
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            left |= right;
            return left;
        }
    };

    value = binaryOperator<Operations>( value, rhs.value );
    ASSERT( isNormalized() ) << "FullRangeInt is not normalized at end of bitwise OR: "<<value;

    return *this;
}

FullRangeInt &FullRangeInt::operator^=( const FullRangeInt &rhs ) {
    struct Operations {
        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughInt right ) {
            left ^= right;
            return left;
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            ABORT()<<"Signed unsigned bitwise XOR";
        }

        static SignedUnsignedValue operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            ABORT()<<"Signed unsigned bitwise XOR";
        }

        static SignedUnsignedValue operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            left ^= right;
            return left;
        }
    };

    value = binaryOperator<Operations>( value, rhs.value );
    normalize();

    return *this;
}

bool operator<( const FullRangeInt &lhs, const FullRangeInt &rhs ) {
    struct Operations {
        static bool operation( LongEnoughInt left, LongEnoughInt right ) {
            return left<right;
        }

        static bool operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            return true;
        }

        static bool operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            return false;
        }

        static bool operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            return left<right;
        }
    };

    return binaryOperator<Operations>( lhs.value, rhs.value );
}

bool operator<=( const FullRangeInt &lhs, const FullRangeInt &rhs ) {
    struct Operations {
        static bool operation( LongEnoughInt left, LongEnoughInt right ) {
            return left<=right;
        }

        static bool operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            return true;
        }

        static bool operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            return false;
        }

        static bool operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            return left<=right;
        }
    };

    return binaryOperator<Operations>( lhs.value, rhs.value );
}

bool operator==( const FullRangeInt &lhs, const FullRangeInt &rhs ) {
    struct Operations {
        static bool operation( LongEnoughInt left, LongEnoughInt right ) {
            return left==right;
        }

        static bool operation( LongEnoughIntSigned left, LongEnoughInt right ) {
            return false;
        }

        static bool operation( LongEnoughInt left, LongEnoughIntSigned right ) {
            return false;
        }

        static bool operation( LongEnoughIntSigned left, LongEnoughIntSigned right ) {
            return left==right;
        }
    };

    return binaryOperator<Operations>( lhs.value, rhs.value );
}

bool operator!=( const FullRangeInt &lhs, const FullRangeInt &rhs ) {
    return !(lhs==rhs);
}

bool operator>( const FullRangeInt &lhs, const FullRangeInt &rhs ) {
    return rhs<=lhs;
}

bool operator>=( const FullRangeInt &lhs, const FullRangeInt &rhs ) {
    return rhs<lhs;
}

std::ostream &operator<<(std::ostream &out, FullRangeInt i) {
    struct Visitor {
        std::ostream &out;

        Visitor( std::ostream &out ) : out(out) {}

        void operator()( LongEnoughInt value ) {
            out<<value;
        }
        void operator()( LongEnoughIntSigned value ) {
            out<<value;
        }
    };

    std::visit( Visitor(out), i.value );
    return out;
}

void FullRangeInt::normalize() {
    struct Visitor {
        SignedUnsignedValue &value;

        Visitor( SignedUnsignedValue &value ) : value(value) {}

        void operator()( LongEnoughInt u ) {
        }
        void operator()( LongEnoughIntSigned s ) {
            if( s>=0 ) {
                value = static_cast<LongEnoughInt>(s);
            }
        }
    };

    std::visit( Visitor(value), value );
}


bool FullRangeInt::isNormalized() const {
    struct Visitor {
        bool operator()( LongEnoughInt u ) {
            return true;
        }
        bool operator()( LongEnoughIntSigned s ) {
            return s<0;
        }
    };

    return std::visit( Visitor(), value );
}
