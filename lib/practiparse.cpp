/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include <iostream>
#include <variant>

#include <unistd.h>

#include "parser.h"
#include "mmap.h"

size_t indentWidth = 3;

std::ostream &indent( std::ostream &out, size_t depth ) {
    for( unsigned i=0; i<depth*indentWidth; ++i )
        out<<" ";

    return out;
}

using namespace NonTerminals;

void dumpIdentifier( const NonTerminals::Identifier &id, size_t depth ) {
    indent(std::cout, depth) << "Identifier "<<*id.identifier<<"\n";
}

void dumpType( const NonTerminals::Type &type, size_t depth ) {
    struct Visitor {
        size_t depth;

        void operator()( std::monostate ) {
            ABORT()<<"Invalid state";
        }

        void operator()( const NonTerminals::Identifier &id ) {
            dumpIdentifier( id, depth );
        }

        void operator()( const NonTerminals::Type::Array &array ) {
            indent( std::cout, depth ) << "Array["<<array.dimension.value<<"]\n";
            dumpType( *array.elementType, depth+1 );
        }

        void operator()( const NonTerminals::Type::Pointer &ptr ) {
            indent( std::cout, depth ) << "Pointer\n";
            dumpType( *ptr.pointed, depth+1 );
        }
    };

    std::visit( Visitor{.depth = depth}, type.type );
}

void dumpParseTree( const NonTerminals::Expression &node, size_t depth=0 );
void dumpParseTree( const NonTerminals::Statement &statement, size_t depth=0 );

void dumpParseTree( const NonTerminals::Statement &statement, size_t depth ) {
    ABORT()<<"TODO implement";
}

void dumpParseTree( const FunctionArguments &args, size_t depth=0 ) {
    for( const auto &i : args.arguments ) {
        dumpParseTree( i, depth+1 );
    }
}

void dumpParseTree( const NonTerminals::Expression &node, size_t depth ) {
    struct Visitor {
        size_t depth;
        std::ostream &out;

        Visitor( size_t depth, std::ostream &out ) : depth(depth), out(out) {}

        void operator()( const std::unique_ptr<::NonTerminals::CompoundExpression> &compound ) {
            indent(out, depth) << "Compound expression Statements:\n";
            for( const auto &statement: compound->statementList.statements ) {
                dumpParseTree( statement, depth+1 );
            }

            indent(out, depth) << "Compound expression value:\n";
                dumpParseTree( compound->expression, depth+1 );
        }

        void operator()( const Literal &literal ) {
            struct Visitor2 {
                const Visitor &_this;

                void operator()( const NonTerminals::LiteralInt &literal ) {
                    indent(_this.out, _this.depth) << "Literal int "<<literal.value<<"\n";
                }

                void operator()( const NonTerminals::LiteralBool &literal ) {
                    indent(_this.out, _this.depth) << "Literal bool "<<(
                            literal.value ?
                            "true" :
                            "false" )<<"\n";
                }

                void operator()( const NonTerminals::LiteralPointer &literal ) {
                    indent(_this.out, _this.depth) << "Literal null\n";
                }

                void operator()( const NonTerminals::LiteralString &literal ) {
                    indent(_this.out, _this.depth) << "Literal string "<<literal.token->text<<"\n";
                }
            };

            std::visit( Visitor2{ ._this = *this }, literal.literal );
        }

        void operator()( const Identifier &id ) {
            dumpIdentifier( id, depth );
        }

        void operator()( const NonTerminals::Expression::UnaryOperator &op ) {
            indent( out, depth )<<"Unary "<<*op.op<<"\n";
            dumpParseTree( *op.operand, depth+1 );
        }

        void operator()( const NonTerminals::Expression::BinaryOperator &op ) {
            indent( out, depth )<<"Binary "<<*op.op<<"\n";
            indent( out, depth )<<"Operand 1:\n";
            dumpParseTree( *op.operands[0], depth+1 );
            indent( out, depth )<<"Operand 2:\n";
            dumpParseTree( *op.operands[1], depth+1 );
        }

        void operator()( const NonTerminals::Expression::CastOperator &op ) {
            indent( out, depth )<<"Cast "<<*op.op<<"\n";
            indent( out, depth )<<"Type:\n";
            dumpType( op.destType, depth+1 );
            indent( out, depth )<<"Expression:\n";
            dumpParseTree( *op.expression, depth+1 );
        }

        void operator()( const NonTerminals::Expression::FunctionCall &func ) {
            indent(out, depth) << "Function call:\n";
            dumpParseTree( *func.expression, depth+1 );
            indent(out, depth) << "Arguments:\n";
            dumpParseTree( func.arguments, depth+1 );
        }

        void operator()( const std::unique_ptr<NonTerminals::ConditionalExpression> &condition ) {
            indent(out, depth) << "Condition expression:\n";
            dumpParseTree( condition->condition, depth+1 );
            indent(out, depth) << "If clause:\n";
            dumpParseTree( condition->ifClause, depth+1 );
            indent(out, depth) << "Else clause:\n";
            dumpParseTree( condition->elseClause, depth+1 );
        }

        void operator()( const NonTerminals::Type &type ) {
            indent(out, depth) << "Type:\n";
            dumpType( type, depth+1 );
        }
    };

    std::visit( Visitor( depth, std::cout ), node.value );
}

void dumpParseTree( const NonTerminals::Module &module, size_t depth=0 ) {
    ABORT()<<"TODO implement";
}

void help() {
    std::cout <<
            "Practiparse: exercise the Practical parser\n"
            "Usage: practiparse filename\n"
            "Options:\n"
            "-c\tArgument is the actual program source, instead of the file name\n"
            "-W\tSource is the whole program, rather than a single expression\n"
            "-i<num>\tSet the per-level indent mount\n";
}

int main(int argc, char *argv[]) {
    bool singleExpression = true;
    bool argumentSource = false;
    int opt;

    while( (opt=getopt(argc, argv, "Wchi:?")) != -1 ) {
        switch( opt ) {
        case 'W':
            singleExpression = false;
            break;
        case 'c':
            argumentSource = true;
            break;
        case 'i':
            indentWidth = strtoul( optarg, nullptr, 10 );
            break;
        case '?':
            help();
            return 0;
        default:
            std::cerr << "Invalid option '-" << static_cast<char>(opt) << "'. Use -? for help." << std::endl;
            help();
            return 1;
        }
    }

    if( optind == argc ) {
        std::cerr << "No argument" << std::endl;
        help();
        return 1;
    }

    try {
        std::unique_ptr< Mmap<MapMode::ReadOnly> > fileSource;
        String textSource;

        if( argumentSource ) {
            textSource = String(argv[optind]);
        } else {
            fileSource = safenew< Mmap<MapMode::ReadOnly> >( (argv[optind]) );
            textSource = fileSource->getSlice<const char>();
        }

        // Tokenize
        auto tokens = Tokenizer::Tokenizer::tokenize( textSource );

        // Parse
        if( singleExpression ) {
            NonTerminals::Expression exp;
            exp.parse( tokens );
            std::cout<<"Successfully parsed. Dumping parse tree:\n";
            dumpParseTree( exp );
        } else {
            NonTerminals::Module module;
            module.parse( tokens );
            std::cout<<"Successfully parsed. Dumping parse tree:\n";
            dumpParseTree( module );
        }
    } catch(std::exception &error) {
        std::cerr << "Parsing failed: " << error.what() << "\n";

        return 1;
    }
}
