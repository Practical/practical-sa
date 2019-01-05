#include "exact_int.h"

#include <cppunit/extensions/HelperMacros.h>

class ExactIntTest : public CppUnit::TestFixture {
    void basicTest() {
        ExactInt u(1u), s(1);

        CPPUNIT_ASSERT( u.getType()!=s.getType() );
        CPPUNIT_ASSERT( u==s );
        unsigned __int128 rawHigh = 1;
        rawHigh <<= 127;
        rawHigh ++;
        ExactInt high( rawHigh ), low( static_cast<signed __int128>(rawHigh) );
        std::cout << high << " " << low << "\n";
        CPPUNIT_ASSERT( high>low );
    }

public:
    static CppUnit::Test *suite()
    {
        CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite( "ExactIntTest" );
        suiteOfTests->addTest( new CppUnit::TestCaller<ExactIntTest>(
                    "basicTest",
                    &ExactIntTest::basicTest ) );
        return suiteOfTests;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( ExactIntTest );
