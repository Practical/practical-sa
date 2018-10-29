#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <variant>
#include <vector>

#include "defines.h"
#include "slice.h"
#include "tokenizer.h"

class parser_error : public compile_error {
public:
    parser_error(const char *msg, size_t line, size_t col) : compile_error(msg, line, col) {
    }
};

namespace Parser {

namespace NonTerminals {
    // Base class for all non-terminals.
    struct NonTerminal {
        Slice<const char> text;
        size_t line=0, col=0;

        // This function is not really virtual. It's used this way to force all children to have the same signature
        // Returns how many tokens were consumed
        // Throws parser_error if fails to parse
        virtual size_t parse(Slice<const Tokenizer::Token> source) = 0;
    };

    struct CompoundExpression;

    struct Expression : public NonTerminal {
        std::variant<std::monostate, std::unique_ptr<CompoundExpression>, Tokenizer::Token> value;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct Statement : public NonTerminal {
        Expression expression;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct StatementList : public NonTerminal {
        std::vector<Statement> statements;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct CompoundExpression : public NonTerminal {
        StatementList statementList;
        Expression expression;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct FuncDeclRet : public NonTerminal {
        Tokenizer::Token type;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct FuncDeclArg : public NonTerminal {
        Tokenizer::Token name, type;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct FuncDeclArgsNonEmpty : public NonTerminal {
        std::vector<FuncDeclArg> arguments;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct FuncDeclArgs : public NonTerminal {
        std::vector<FuncDeclArg> arguments;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct FuncDeclBody : public NonTerminal {
        Tokenizer::Token name;
        FuncDeclArgs arguments;
        FuncDeclRet returnType;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct FuncDef : public NonTerminal {
        FuncDeclBody decl;
        CompoundExpression body;

        size_t parse(Slice<const Tokenizer::Token> source) override final;

        String name() const {
            return decl.name.text;
        }
    };

    struct Module : public NonTerminal {
        std::vector< FuncDef > functionDefinitions;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };
}

} // namespace Parser

#endif // PARSER_H
