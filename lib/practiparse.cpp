#include <iostream>
#include <variant>

#include <unistd.h>

#include "parser.h"
#include "mmap.h"

std::ostream &indent( std::ostream &out, size_t depth ) {
    for( unsigned i=0; i<depth; ++i )
        out<<" ";

    return out;
}

std::ostream &operator<<( std::ostream &out, const Tokenizer::Token &token ) {
    out<<token.token<<" ("<<token.text<<") at "<<token.line<<":"<<token.col;
    return out;
}
using namespace NonTerminals;

void dumpParseTree( const Expression &node, size_t depth=0 );

void dumpParseTree( const FunctionArguments &node, size_t depth=0 ) {
}

void dumpParseTree( const Expression &node, size_t depth ) {
    struct Visitor {
        size_t depth;
        std::ostream &out;

        Visitor( size_t depth, std::ostream &out ) : depth(depth), out(out) {}

        void operator()( const std::unique_ptr<::NonTerminals::CompoundExpression> &compound ) {}
        void operator()( const Literal &literal ) {
            indent(out, depth) << "Literal "<<literal.token<<"\n";
        }

        void operator()( const Identifier &id ) {
            indent(out, depth) << "Identifier "<<*id.identifier<<"\n";
        }

        void operator()( const Expression::UnaryOperator &op ) {
            indent( out, depth )<<"Unary "<<*op.op<<"\n";
            dumpParseTree( *op.operand, depth+1 );
        }

        void operator()( const Expression::BinaryOperator &op ) {
            indent( out, depth )<<"Binary "<<*op.op<<"\n";
            indent( out, depth )<<"Operand 1:\n";
            dumpParseTree( *op.operand1, depth+1 );
            indent( out, depth )<<"Operand 2:\n";
            dumpParseTree( *op.operand2, depth+1 );
        }

        void operator()( const Expression::FunctionCall &func ) {
            indent(out, depth) << "Function call:\n";
            dumpParseTree( *func.expression, depth+1 );
            indent(out, depth) << "Arguments:\n";
            dumpParseTree( func.arguments, depth+1 );
        }
    };

    std::visit( Visitor( depth, std::cout ), node.value );
}

void help() {
    std::cout <<
            "Practiparse: exercise the Practical parser\n"
            "Usage: practiparse filename\n"
            "Options:\n"
            "-c\tArgument is the actual program source, instead of the file name\n"
            "-E\tSource is a single expression, rather than the whole program\n";
}

int main(int argc, char *argv[]) {
    bool singleExpression;
    bool argumentSource;
    int opt;

    while( (opt=getopt(argc, argv, "Ech")) != -1 ) {
        switch( opt ) {
        case 'E':
            singleExpression = true;
            break;
        case 'c':
            argumentSource = true;
            break;
        case '?':
            help();
            return 1;
        default:
            std::cerr << "Invalid branch of case reached with option '" << opt << "'. Aborting" << std::endl;
            abort();
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
        }
    } catch(std::exception &error) {
        std::cerr << "Parsing failed: " << error.what() << "\n";

        return 1;
    }
}
