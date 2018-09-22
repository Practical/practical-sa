#include <string.h>

#include <cppunit/extensions/HelperMacros.h>

#include "slice.h"

class SliceTest : public CppUnit::TestFixture  {
    void basicTest() {
#define TEST_STR "Hello, world"
        char string[] = TEST_STR;
        auto slice = Slice<char>(string, sizeof(TEST_STR));
        CPPUNIT_ASSERT( slice.length()==13 );
        CPPUNIT_ASSERT( slice[0]=='H' );
        CPPUNIT_ASSERT( slice[4]=='o' );
        slice[1] = 'a';
        CPPUNIT_ASSERT( strcmp(string, "Hallo, world")==0 );
        CPPUNIT_ASSERT( slice[12] == '\0' );

        auto subslice = static_cast<const Slice<char> &>(slice).subslice(1, 11);
        CPPUNIT_ASSERT( subslice[1]=='l' );
        CPPUNIT_ASSERT( subslice[subslice.length()-1]=='r' );
        CPPUNIT_ASSERT( subslice.length()==9 );

        auto mutableSubslice = slice.subslice(2, 12);
        CPPUNIT_ASSERT( mutableSubslice[1]=='l' );
        CPPUNIT_ASSERT( mutableSubslice[subslice.length()-1]=='l' );
        CPPUNIT_ASSERT( mutableSubslice.length()==9 );

    }

public:
    static CppUnit::Test *suite()
    {
        CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite( "SliceTest" );
        suiteOfTests->addTest( new CppUnit::TestCaller<SliceTest>(
                    "basicTest",
                    &SliceTest::basicTest ) );
        return suiteOfTests;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( SliceTest );
