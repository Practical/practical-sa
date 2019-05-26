/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "full_range_int.h"

#include <cppunit/extensions/HelperMacros.h>

class FullRangeIntTest : public CppUnit::TestFixture  {
    void basicTest() {
        FullRangeInt a;
        FullRangeInt b(12);
        FullRangeInt c(-24);

        CPPUNIT_ASSERT( a-(b+c) == b );

        a=0x0100'0000'0000'0000;
        c=a;
        b = -a;
        for( int i=0; i<56; ++i ) {
            a/=2;
        }
        CPPUNIT_ASSERT(a==1);
        CPPUNIT_ASSERT(b<0);
        CPPUNIT_ASSERT( c<INTMAX_MAX );
        c*=0x80;
        CPPUNIT_ASSERT( c>INTMAX_MAX );
        CPPUNIT_ASSERT( c>b );
        a=UINTMAX_MAX;
        CPPUNIT_ASSERT( a>c );
        CPPUNIT_ASSERT( a>b );
    }
public:
    static CppUnit::Test *suite()
    {
        CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite( "FullRangeInt test" );
        suiteOfTests->addTest( new CppUnit::TestCaller<FullRangeIntTest>(
                    "basicTest",
                    &FullRangeIntTest::basicTest ) );
        return suiteOfTests;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( FullRangeIntTest );
