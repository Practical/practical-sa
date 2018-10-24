#include <string.h>

#include <cppunit/extensions/HelperMacros.h>

#include "slice.h"

#include <unordered_map>

class SliceTest : public CppUnit::TestFixture  {
    void basicTest() {
#define TEST_STR "Hello, world"
        char string[] = TEST_STR;
        auto slice = Slice<char>(string, sizeof(TEST_STR));
        CPPUNIT_ASSERT( slice.size()==13 );
        CPPUNIT_ASSERT( slice[0]=='H' );
        CPPUNIT_ASSERT( slice[4]=='o' );
        slice[1] = 'a';
        CPPUNIT_ASSERT( strcmp(string, "Hallo, world")==0 );
        CPPUNIT_ASSERT( slice[12] == '\0' );

        auto subslice = static_cast<const Slice<char> &>(slice).subslice(1, 10);
        CPPUNIT_ASSERT( subslice[1]=='l' );
        CPPUNIT_ASSERT( subslice[subslice.size()-1]=='r' );
        CPPUNIT_ASSERT( subslice.size()==9 );

        auto mutableSubslice = slice.subslice(2, 11);
        CPPUNIT_ASSERT( mutableSubslice[1]=='l' );
        CPPUNIT_ASSERT( mutableSubslice[subslice.size()-1]=='l' );
        CPPUNIT_ASSERT( mutableSubslice.size()==9 );

    }

    void tokenizerFailure() {
        static const char testString[] = "Hello\nhi\n";
        auto slice = Slice<const char>(testString, strlen(testString));

        size_t i = 5;
        auto sub1 = slice.subslice(0, i);
        auto sub2 = slice.subslice(i+1);

        CPPUNIT_ASSERT( sub1.size()==5 );
        CPPUNIT_ASSERT( sub1[0]=='H' );
        CPPUNIT_ASSERT( sub1[4]=='o' );
        CPPUNIT_ASSERT( sub2.size()==3 );
        CPPUNIT_ASSERT( sub2[0]=='h' );
        CPPUNIT_ASSERT( sub2[2]=='\n' );
    }

    void hashTest() {
        std::unordered_map<Slice<const char>, int> map;

        char buffer[] = "1234567891";
        Slice<const char> s(buffer, 1);
        std::hash<Slice<const char>> hasher;
        size_t hash1 = hasher(s);

        for( size_t i=0; i<sizeof(buffer); ++i ) {
            s = Slice<const char>(buffer+i, 1);
            map[s] = i;
        }
        for( size_t i=0; i<sizeof(buffer)-1; ++i ) {
            s = Slice<const char>(buffer+i, 2);
            map[s] = i + 20;
        }

        CPPUNIT_ASSERT( hash1 != static_cast<unsigned int>(hash1) );
        s = Slice<const char>(buffer+9, 1);
        CPPUNIT_ASSERT( hash1 == hasher(s) );

        s = Slice<const char>("23", 2);
        CPPUNIT_ASSERT( map[s] == 21 );
    }
public:
    static CppUnit::Test *suite()
    {
        CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite( "SliceTest" );
        suiteOfTests->addTest( new CppUnit::TestCaller<SliceTest>(
                    "basicTest",
                    &SliceTest::basicTest ) );
        suiteOfTests->addTest( new CppUnit::TestCaller<SliceTest>(
                    "tokenizerFailure",
                    &SliceTest::tokenizerFailure ) );
        suiteOfTests->addTest( new CppUnit::TestCaller<SliceTest>(
                    "hashTest",
                    &SliceTest::hashTest ) );
        return suiteOfTests;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( SliceTest );
