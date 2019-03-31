/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "tokenizer.h"

#include "mmap.h"
#include "practical-errors.h"
#include "ut/dirscan.h"

#include <cppunit/extensions/HelperMacros.h>

#include <algorithm>
#include <regex>

class TokenizerTest : public CppUnit::TestFixture  {
    static constexpr size_t TEST_NAME_MAX_LENGTH = 200;
    struct ExpectedToken {
        size_t line, column, offset;
        Tokenizer::Tokens token;
    };

    template <typename T>
    static Slice<T> consumeLine(Slice<T> &buffer) {
        if( buffer.size()==0 )
            throw std::runtime_error("Buffer fully consumed");

        size_t i = 0;
        while( i<buffer.size() && buffer[i]!='\n' )
            ++i;

        if( i==buffer.size() ) {
            auto result = buffer;
            buffer = Slice<T>();

            return result;
        }

        Slice<T> result;

        result = buffer.subslice(0, i);
        buffer = buffer.subslice(i+1);

        return result;
    }

    void runTest(std::string &file) {
        Mmap<MapMode::CopyOnWrite> testPlanMap(file);
        Slice<char> testPlan = testPlanMap.getSlice<char>();

        file.resize(file.size() - 3);
        Mmap<MapMode::ReadOnly> testDataMap(file);
        Slice<const char> testData = testDataMap.getSlice<const char>();

        auto testName = consumeLine(testPlan);

        std::regex csvParse("([A-Z0-9_]+),(\\d+),(\\d+)");

        std::cout<<testName<<"\n";

        struct TestPoint {
            Tokenizer::Tokens token;
            size_t line, column;
        };
        std::vector<TestPoint> testPoints;
        size_t finishLine, finishCol;

        bool done = false;
        while( !done && testPlan.size()>0 ) {
            auto line = consumeLine(testPlan);
            std::smatch parsedFields;
            std::string l(line.get(), line.size());
            if( std::regex_match( l, parsedFields, csvParse ) ) {
                TestPoint point;

                size_t line = strtoul( parsedFields[2].str().c_str(), nullptr, 0 );
                size_t col = strtoul( parsedFields[3].str().c_str(), nullptr, 0 );

#define CASE(name) if( parsedFields[1].str()==#name ) {\
                    point.token = Tokenizer::Tokens::name;\
                    point.line = line; point.column = col;\
                    testPoints.push_back(point);\
                }
                CASE(ERR)
                else CASE(WS) // White space
                else CASE(COMMENT_LINE_END)
                else CASE(COMMENT_MULTILINE)
                // Syntax separators
                else CASE(SEMICOLON)
                else CASE(COMMA)
                // Bracket types
                else CASE(BRACKET_ROUND_OPEN)
                else CASE(BRACKET_ROUND_CLOSE)
                else CASE(BRACKET_SQUARE_OPEN)
                else CASE(BRACKET_SQUARE_CLOSE)
                else CASE(BRACKET_CURLY_OPEN)
                else CASE(BRACKET_CURLY_CLOSE)
                // Operators
                else CASE(OP_AMPERSAND)
                else CASE(OP_ARROW)                                   // ->
                else CASE(OP_ASSIGN)
                else CASE(OP_ASSIGN_BIT_AND)
                else CASE(OP_ASSIGN_BIT_OR)
                else CASE(OP_ASSIGN_BIT_XOR)
                else CASE(OP_ASSIGN_DIVIDE)
                else CASE(OP_ASSIGN_LEFT_SHIFT)
                else CASE(OP_ASSIGN_MINUS)
                else CASE(OP_ASSIGN_MODULOUS)
                else CASE(OP_ASSIGN_MULTIPLY)
                else CASE(OP_ASSIGN_PLUS)
                else CASE(OP_ASSIGN_RIGHT_SHIFT)
                else CASE(OP_ASSIGN_RIGHT_SHIFT_LOGICAL)
                else CASE(OP_ASTERISK)
                else CASE(OP_BIT_AND)
                else CASE(OP_BIT_NOT)
                else CASE(OP_BIT_OR)
                else CASE(OP_BIT_XOR)
                else CASE(OP_COLON)
                else CASE(OP_DIVIDE)
                else CASE(OP_DOT)
                else CASE(OP_DOUBLE_COLON)
                else CASE(OP_PAAMAYIM_NEKUDATAYIM)
                else CASE(OP_EQUALS)
                else CASE(OP_GREATER_THAN)
                else CASE(OP_GREATER_THAN_EQ)
                else CASE(OP_LESS_THAN)
                else CASE(OP_LESS_THAN_EQ)
                else CASE(OP_LOGIC_AND)
                else CASE(OP_LOGIC_NOT)
                else CASE(OP_LOGIC_OR)
                else CASE(OP_MINUS)
                else CASE(OP_MINUS_MINUS)
                else CASE(OP_MODULOUS)
                else CASE(OP_NOT_EQUALS)
                else CASE(OP_PLUS)
                else CASE(OP_PLUS_PLUS)
                else CASE(OP_SHIFT_LEFT)
                else CASE(OP_SHIFT_RIGHT)
                else CASE(OP_SHIFT_RIGHT_LOGICAL)
                else CASE(OP_RUNON_ERROR)
                // Literals
                else CASE(LITERAL_INT_2)
                else CASE(LITERAL_INT_8)
                else CASE(LITERAL_INT_10)
                else CASE(LITERAL_INT_16)
                else CASE(LITERAL_FP)
                else CASE(LITERAL_STRING)
                // Identifier like
                else CASE(IDENTIFIER)
                else CASE(RESERVED_DEF)
                else if( parsedFields[1].str()=="END" ) {
                    done = true;
                    finishLine = line;
                    finishCol = col;
                } else {
                    std::cerr<<"Requested unknown token \""<<parsedFields[1].str()<<"\"\n";
                    CPPUNIT_FAIL("Test plan requested an unknown token");
                }
#undef CASE
            } else {
                std::cout << "No match line: \"" << l << "\" Len: " << line.size() << " buffer: " << testPlan.size() << "\n";
                CPPUNIT_ASSERT(false);
            }
        }

        size_t expectedIndex = 0;
        Tokenizer::Tokenizer tokenizer(testData);
        while( true ) {
            TestPoint *point = &testPoints[expectedIndex];

            Tokenizer::Tokens currentToken;
            size_t currentLine, currentCol;
            try {
                if( !tokenizer.next() )
                    break;

                currentToken = tokenizer.currentToken();
                currentLine = tokenizer.currentLine();
                currentCol = tokenizer.currentCol();
            } catch( PracticalSemanticAnalyzer::tokenizer_error &ex ) {
                currentToken = Tokenizer::Tokens::ERR;
                currentLine = ex.getLine();
                currentCol = ex.getCol();
            }
            std::cout<<"Tokenizer matched "<<currentToken<<"\n";

            if( currentToken == point->token ) {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected token line", point->line, currentLine );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected token column", point->column, currentCol );
                expectedIndex++;
            } else if( currentToken==Tokenizer::Tokens::WS ) {
                // Do nothing: we're allowed to ignore white spaces
            } else {
                std::cerr << "At " << currentLine << ":" << currentCol << ": Detected token " << currentToken <<
                        ", expected " << point->token << " " << point->line << ":" << point->column << ": text was \"" <<
                        tokenizer.currentTokenText() << "\"\n";
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Tokenizer returned unexpected token", point->token, currentToken);
                CPPUNIT_FAIL("Unreachable code reached");
            }
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not all expected tokens were matched", testPoints.size(), expectedIndex );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Tokenizer finish line incorrect", finishLine, tokenizer.currentLine() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Tokenizer finish column incorrect", finishCol, tokenizer.currentCol() );
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
