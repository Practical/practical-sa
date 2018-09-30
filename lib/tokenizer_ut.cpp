#include <algorithm>
#include <regex>

#include <cppunit/extensions/HelperMacros.h>

#include "tokenizer.h"
#include "ut/dirscan.h"
#include "mmap.h"

class TokenizerTest : public CppUnit::TestFixture  {
    static constexpr size_t TEST_NAME_MAX_LENGTH = 200;
    struct ExpectedToken {
        size_t line, column, offset;
        Tokenizer::Tokens token;
    };

    template <typename T>
    static Slice<T> consumeLine(Slice<T> &buffer) {
        if( buffer.length()==0 )
            throw std::runtime_error("Buffer fully consumed");

        size_t i = 0;
        while( i<buffer.length() && buffer[i]!='\n' )
            ++i;

        if( i==buffer.length() ) {
            auto result = buffer;
            buffer = Slice<T>();

            return result;
        }

        Slice<T> result;

        result = buffer.subslice(0, i+1);
        buffer = buffer.subslice(i+1);

        return result;
    }

    void runTest(std::string &file) {
        Mmap<MapMode::CopyOnWrite> testPlanMap(file);
        Slice<char> testPlan = testPlanMap.getSlice<char>();

        file.resize(file.length() - 3);
        Mmap<MapMode::ReadOnly> testDataMap(file);
        Slice<const char> testData = testDataMap.getSlice<const char>();

        auto testName = consumeLine(testPlan);

        std::regex csvParse("([A-Z]+),(\\d+),(\\d+)");

        std::cout<<testName<<"\n";

        struct TestPoint {
            Tokenizer::Tokens token;
            size_t line, column;
        };
        std::vector<TestPoint> testPoints;
        size_t finishLine, finishCol;

        bool done = false;
        while( !done && testPlan.length()>0 ) {
            auto line = consumeLine(testPlan);
            std::smatch parsedFields;
            std::string l(line.get(), line.length());
            if( std::regex_match( l, parsedFields, csvParse ) ) {
                TestPoint point;

                size_t line = strtoul( parsedFields[2].str().c_str(), nullptr, 0 );
                size_t col = strtoul( parsedFields[3].str().c_str(), nullptr, 0 );

#define CASE(name) if( parsedFields[1].str()==#name ) {\
                    point.token = Tokenizer::Tokens::name;\
                    point.line = line; point.column = col;\
                    testPoints.push_back(point);\
                }
                CASE(WS)
                else CASE(OPEN_ROUND_BRACKET)
                else if( parsedFields[1].str()=="END" ) {
                    done = true;
                    finishLine = line;
                    finishCol = col;
                } else {
                    std::cerr<<"Requested unknown token \""<<parsedFields[1].str()<<"\"\n";
                    CPPUNIT_ASSERT(false);
                }
#undef CASE
            } else {
                std::cout << "No match line: " << l << " Len: " << line.length() << " buffer: " << testPlan.length() << "\n";
            }
        }

        size_t expectedIndex = 0;
        Tokenizer tokenizer(testData);
        while( !tokenizer.done() ) {
            TestPoint *point = &testPoints[expectedIndex];
            if( tokenizer.token() == point->token ) {
                CPPUNIT_ASSERT( tokenizer.currentLine() == point->line );
                CPPUNIT_ASSERT( tokenizer.currentCol() == point->column );
                expectedIndex++;
                tokenizer.next();
            } else if( tokenizer.token()==Tokenizer::Tokens::WS ) {
                // Do nothing: we're allowed to ignore white spaces
                tokenizer.next();
            } else {
                std::cerr << "At " << tokenizer.currentLine() << ":" << tokenizer.currentCol() << ": Detected token " << tokenizer.token() <<
                        ", expected " << point->token << " " << point->line << ":" << point->column << ": text was \"" <<
                        tokenizer.currentTokenText() << "\"\n";
                CPPUNIT_ASSERT(false);
            }
        }

        CPPUNIT_ASSERT( tokenizer.currentLine()==finishLine );
        CPPUNIT_ASSERT( tokenizer.currentCol()==finishCol );
    }

    void test() {
        const char *basePath = getenv("TOP_DIR");
        std::string path;
        if( basePath!=nullptr ) {
            path = basePath;
            path += '/';
        }
        path += "tests/tokenizer/";

        for(auto &i: DirScan(path, ".ut")) {
            std::cout << "Running " << i.d_name << " test\n";
            std::string file(path);
            file+=i.d_name;
            runTest(file);
        }
    }

public:
    static CppUnit::Test *suite()
    {
        CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite( "TokenizerTest" );
        suiteOfTests->addTest( new CppUnit::TestCaller<TokenizerTest>(
                    "test",
                    &TokenizerTest::test ) );
        return suiteOfTests;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( TokenizerTest );
