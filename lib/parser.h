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
        virtual size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) = 0;
    };

    struct Type : public NonTerminal {
        Tokenizer::Token type;
        IdentifierId identId;

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;
        void symbolsPass2(const LookupContext *parent);
    };

    struct CompoundExpression;

    struct Expression : public NonTerminal {
        std::variant<std::monostate, std::unique_ptr<CompoundExpression>, Tokenizer::Token> value;

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;
    };

    struct Statement : public NonTerminal {
        Expression expression;

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;
    };

    struct StatementList : public NonTerminal {
        std::vector<Statement> statements;

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;
    };

    struct CompoundExpression : public NonTerminal {
        StatementList statementList;
        Expression expression;
        LookupContext context;

        CompoundExpression(const LookupContext *parent) : context(parent) {
        }

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;
    };

    struct FuncDeclRet : public NonTerminal {
        Type type;

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;
        void symbolsPass2(const LookupContext *ctx);
    };

    struct FuncDeclArg : public NonTerminal {
        Tokenizer::Token name;
        Type type;

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;
    };

    struct FuncDeclArgsNonEmpty : public NonTerminal {
        std::vector<FuncDeclArg> arguments;

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;
    };

    struct FuncDeclArgs : public NonTerminal {
        std::vector<FuncDeclArg> arguments;

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;
    };

    struct FuncDeclBody : public NonTerminal {
        Tokenizer::Token name;
        FuncDeclArgs arguments;
        FuncDeclRet returnType;

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;
        void symbolsPass2(const LookupContext *ctx);
    };

    struct FuncDef : public NonTerminal {
        FuncDeclBody decl;
        CompoundExpression body;
        PracticalSemanticAnalyzer::IdentifierId id;

        FuncDef(const LookupContext *ctx) : body{ctx} {
        }

        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;

        String name() const {
            return decl.name.text;
        }

        void symbolsPass2(const LookupContext *ctx);

        void codeGen(PracticalSemanticAnalyzer::CodeGen *codeGen);
    };

    struct Module : public NonTerminal {
        PracticalSemanticAnalyzer::ModuleId id;
        std::vector< FuncDef > functionDefinitions;
        std::vector< Tokenizer::Token > tokens;
        LookupContext context;

        Module(LookupContext *parent) : context(parent) {
        }

        void parse(String source);
        size_t parse(Slice<const Tokenizer::Token> source, const LookupContext *ctx) override final;

        void symbolsPass1(LookupContext *parent) {
            context.symbolsPass1(functionDefinitions);
        }

        void symbolsPass2(LookupContext *parent) {
            context.symbolsPass2(functionDefinitions);
        }

        void codeGen(PracticalSemanticAnalyzer::CodeGen *codeGen);
    };
} // NonTerminals namespace

#endif // PARSER_H
