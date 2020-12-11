/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PARSER_H
#define PARSER_H

#include "parser/literal_bool.h"
#include "parser/literal_int.h"
#include "parser/literal_string.h"

#include "asserts.h"
#include "operators.h"

#include <practical/defines.h>
#include <practical/practical.h>
#include <practical/slice.h>

#include <memory>
#include <variant>
#include <vector>

using namespace PracticalSemanticAnalyzer;

namespace NonTerminals {
    // Base class for all non-terminals.
    struct Identifier : public NonTerminal {
        const Tokenizer::Token *identifier = nullptr;

        size_t parse(Slice<const Tokenizer::Token> source) override final;

        String getName() const {
            return identifier->text;
        }

        SourceLocation getLocation() const {
            ASSERT(identifier != nullptr) << "Dereferencing an unparsed identifier";
            return identifier->location;
        }
    };

    struct Type : public NonTerminal {
        struct Pointer {
            std::unique_ptr< const Type > pointed;
            const Tokenizer::Token *token = nullptr;

            Pointer( std::unique_ptr< const Type > pointed, const Tokenizer::Token *token ) :
                pointed(std::move(pointed)), token(token)
            {}
        };
        std::variant<std::monostate, Identifier, Pointer> type;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
        SourceLocation getLocation() const;
    };

    struct TransientType : public NonTerminal {
        Type type;
        const Tokenizer::Token *ref = nullptr;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct LiteralPointer : public NonTerminal {
        const Tokenizer::Token *token = nullptr;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct Literal : public NonTerminal {
        std::variant<LiteralInt, LiteralBool, LiteralPointer, LiteralString> literal;

        size_t parse(Slice<const Tokenizer::Token> source) override final;

        SourceLocation getLocation() const;
    };

    struct Expression;
    struct FunctionArguments : public NonTerminal {
        std::vector<Expression> arguments;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct CompoundExpression;

    struct ConditionalExpression;
    struct Expression : public NonTerminal {
        struct UnaryOperator {
            const Tokenizer::Token *op;
            std::unique_ptr<Expression> operand;
        };

        struct BinaryOperator {
            const Tokenizer::Token *op;
            std::array< std::unique_ptr<Expression>, 2 > operands;
        };

        struct CastOperator {
            const Tokenizer::Token *op;
            Type destType;
            std::unique_ptr<Expression> expression;
        };

        struct FunctionCall {
            const Tokenizer::Token *op;
            std::unique_ptr<Expression> expression;
            FunctionArguments arguments;
        };

        std::variant<
                std::unique_ptr<::NonTerminals::CompoundExpression>,
                ::NonTerminals::Literal,
                Identifier,
                UnaryOperator,
                BinaryOperator,
                CastOperator,
                FunctionCall,
                std::unique_ptr<ConditionalExpression>,
                Type
            > value;
    private:
        mutable std::unique_ptr<Type> altTypeParse;

    public:
        Expression() {}
        explicit Expression( ConditionalExpression &&condition ) :
            value( safenew<ConditionalExpression>( std::move(condition) ) )
        {}
        Expression( Expression &&that ) : value( std::move(that.value) ), altTypeParse( std::move(that.altTypeParse) )
        {}
        Expression &operator=( Expression &&that ) {
            value = std::move( that.value );
            altTypeParse = std::move( that.altTypeParse );

            return *this;
        }

        explicit Expression( std::unique_ptr<CompoundExpression> &&compoundExpression ) :
            value( std::move(compoundExpression) )
        {}

        size_t parse(Slice<const Tokenizer::Token> source) override final;
        const Type *reparseAsType() const;

    private:
        size_t actualParse(Slice<const Tokenizer::Token> source, size_t level);
        size_t basicParse(Slice<const Tokenizer::Token> source);

        size_t parsePrefixOp(
                Slice<const Tokenizer::Token> source, size_t level, const Operators::OperatorPriority::OperatorsMap &operators);
        size_t parseInfixOp(
                Slice<const Tokenizer::Token> source, size_t level, const Operators::OperatorPriority::OperatorsMap &operators);
        size_t parseInfixR2LOp(
                Slice<const Tokenizer::Token> source, size_t level, const Operators::OperatorPriority::OperatorsMap &operators);
        size_t parsePostfixOp(
                Slice<const Tokenizer::Token> source, size_t level, const Operators::OperatorPriority::OperatorsMap &operators);
    };

    struct ConditionalExpression {
        Expression condition;
        Expression ifClause, elseClause;
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

    struct CompoundStatement;

    struct Statement : public NonTerminal {
        struct ConditionalStatement {
            Expression condition;
            std::unique_ptr<Statement> ifClause, elseClause;
        };

        Statement() {}
        explicit Statement( std::unique_ptr<CompoundStatement> &&compoundStatement ) :
            content( std::move(compoundStatement) )
        {}
        explicit Statement( Expression &&expression ) : content( std::move(expression) ) {}

        std::variant<
                std::monostate,
                Expression,
                VariableDefinition,
                ConditionalStatement,
                std::unique_ptr<CompoundStatement>
            > content;

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
        CompoundExpression( StatementList &&statements, Expression &&expression ) :
            statementList( std::move(statements) ),
            expression( std::move(expression) )
        {}
        CompoundExpression( CompoundExpression &&that ) :
            statementList( std::move(that.statementList) ),
            expression( std::move(that.expression) )
        {}

        CompoundExpression &operator=( CompoundExpression &&that ) {
            statementList = std::move( that.statementList );
            expression = std::move( that.expression );

            return *this;
        }

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct CompoundStatement : public NonTerminal {
        StatementList statements;

        CompoundStatement() {}
        CompoundStatement( StatementList &&statements ) : statements( std::move(statements) ) {}

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct FuncDeclRet : public NonTerminal {
        TransientType type;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
    };

    struct FuncDeclArg : public NonTerminal {
        Identifier name;
        TransientType type;

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
        std::variant<std::monostate, CompoundExpression, CompoundStatement> body;

        FuncDef() : body{} {
        }
        FuncDef( FuncDef &&that ) : decl( std::move(that.decl) ), body( std::move(that.body) ) {}

        size_t parse(Slice<const Tokenizer::Token> source) override final;

        String getName() const {
            return decl.name.getName();
        }
    };

    struct FuncDecl : public NonTerminal {
        FuncDeclBody decl;
        LiteralString abiSpecifier;

        FuncDecl() {}
        FuncDecl( FuncDecl &&that ) = default;

        size_t parse(Slice<const Tokenizer::Token> source) override final;

        String getName() const {
            return decl.name.getName();
        }
    };

    struct Module : public NonTerminal {
        std::vector< FuncDef > functionDefinitions;
        std::vector< FuncDecl > functionDeclarations;
        std::vector< Tokenizer::Token > tokens;

        void parse(String source);
        size_t parse(Slice<const Tokenizer::Token> source) override final;
        String getName() const {
            return toSlice("__main");
        }

    };
} // NonTerminals namespace

#endif // PARSER_H
