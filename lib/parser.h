#ifndef PARSER_H
#define PARSER_H

#include "defines.h"
#include "lookup_context.h"
#include "practical-sa.h"
#include "slice.h"
#include "tokenizer.h"

#include <memory>
#include <variant>
#include <vector>

using namespace PracticalSemanticAnalyzer;

class parser_error : public compile_error {
public:
    parser_error(const char *msg, size_t line, size_t col) : compile_error(msg, line, col) {
    }
};

class pass1_error : public compile_error {
public:
    pass1_error(const char *msg, size_t line, size_t col) : compile_error(msg, line, col) {
    }
};

class pass2_error : public compile_error {
public:
    pass2_error(const char *msg, size_t line, size_t col) : compile_error(msg, line, col) {
    }
};

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

    struct Type : public NonTerminal {
        Tokenizer::Token type;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
        size_t getLine() const {
            return type.line;
        }
        size_t getCol() const {
            return type.col;
        }
    };

    struct Literal : public NonTerminal {
        Tokenizer::Token token;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct CompoundExpression;

    struct Expression : public NonTerminal {
        std::variant<
                std::monostate,
                std::unique_ptr<::NonTerminals::CompoundExpression>,
                ::NonTerminals::Literal,
                const Tokenizer::Token *
            > value;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct VariableDeclBody : public NonTerminal {
        const Tokenizer::Token *name;
        Type type;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct VariableDefinition : public NonTerminal {
        VariableDeclBody body;
        std::unique_ptr<Expression> initValue;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct Statement : public NonTerminal {
        std::variant<std::monostate, Expression, VariableDefinition> content;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct StatementList : public NonTerminal {
        std::vector<Statement> statements;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct CompoundExpression : public NonTerminal {
        StatementList statementList;
        Expression expression;

        CompoundExpression() {
        }

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct FuncDeclRet : public NonTerminal {
        Type type;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct FuncDeclArg : public NonTerminal {
        Tokenizer::Token name;
        Type type;

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

        FuncDef() : body{} {
        }

        size_t parse(Slice<const Tokenizer::Token> source) override final;

        String getName() const {
            return decl.name.text;
        }
    };

    struct Module : public NonTerminal {
        std::vector< FuncDef > functionDefinitions;
        std::vector< Tokenizer::Token > tokens;

        void parse(String source);
        size_t parse(Slice<const Tokenizer::Token> source) override final;
        String getName() const {
            return toSlice("__main");
        }

    };
} // NonTerminals namespace

#endif // PARSER_H
