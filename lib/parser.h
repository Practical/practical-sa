#ifndef PARSER_H
#define PARSER_H

#include "asserts.h"
#include "defines.h"
#include "lookup_context.h"
#include "practical-sa.h"
#include "slice.h"
#include "tokenizer.h"

#include <memory>
#include <variant>
#include <vector>

using namespace PracticalSemanticAnalyzer;

namespace NonTerminals {
    // Base class for all non-terminals.
    struct NonTerminal {
        // This function is not really virtual. It's used this way to force all children to have the same signature
        // Returns how many tokens were consumed
        // Throws parser_error if fails to parse
        virtual size_t parse(Slice<const Tokenizer::Token> source) = 0;

        virtual ~NonTerminal() {}
    };

    struct Identifier : public NonTerminal {
        const Tokenizer::Token *identifier = nullptr;

        size_t parse(Slice<const Tokenizer::Token> source) override final;

        String getName() const {
            return identifier->text;
        }

        size_t getLine() const {
            ASSERT(identifier != nullptr) << "Dereferencing an unparsed identifier";
            return identifier->line;
        }

        size_t getCol() const {
            ASSERT(identifier != nullptr) << "Dereferencing an unparsed identifier";
            return identifier->col;
        }
    };

    struct Type : public NonTerminal {
        Identifier type;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
        size_t getLine() const {
            return type.getLine();
        }
        size_t getCol() const {
            return type.getCol();
        }
    };

    struct Literal : public NonTerminal {
        Tokenizer::Token token;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct Expression;
    struct FunctionArguments : public NonTerminal {
        std::vector<Expression> arguments;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct CompoundExpression;

    struct Expression : public NonTerminal {
        struct FunctionCall {
            std::unique_ptr<Expression> expression;
            FunctionArguments arguments;
        };

        std::variant<
                std::monostate,
                std::unique_ptr<::NonTerminals::CompoundExpression>,
                ::NonTerminals::Literal,
                Identifier,
                FunctionCall
            > value;

        size_t parse(Slice<const Tokenizer::Token> source) override final;

    private:
        size_t actualParse(Slice<const Tokenizer::Token> source);
    };

    struct VariableDeclBody : public NonTerminal {
        Identifier name;
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
        Identifier name;
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
        Identifier name;
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
            return decl.name.getName();
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
