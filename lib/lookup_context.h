#ifndef LOOKUP_CONTEXT_H
#define LOOKUP_CONTEXT_H

#include "nocopy.h"
#include "tokenizer.h"

#include <practical-sa.h>

// A Context is anything that may contain further symbolic definitions. This may be a module, a struct, a function or even an
// anonymous block of code.
class LookupContext : private NoCopy {
public:
    class NamedType : public PracticalSemanticAnalyzer::NamedType, private NoCopy {
    private:
        PracticalSemanticAnalyzer::TypeId _id;
        size_t _size;
        const Tokenizer::Token *_name;
        Type _type;

    public:
        NamedType(const Tokenizer::Token *name, NamedType::Type type, size_t size);
        NamedType(NamedType &&other);
        NamedType &operator=(NamedType &&other);

        size_t size() const override {
            return _size;
        }

        String name() const override {
            return _name->text;
        }

        Type type() const override {
            return _type;
        }

        PracticalSemanticAnalyzer::TypeId id() const override {
            return _id;
        }
    };

    class LocalVariable {
    public:
        PracticalSemanticAnalyzer::StaticType type;
        const Tokenizer::Token *name;
        PracticalSemanticAnalyzer::IdentifierId id;
        PracticalSemanticAnalyzer::ExpressionId lvalueId;

        explicit LocalVariable( const Tokenizer::Token *name );
        LocalVariable(
                const Tokenizer::Token *name, PracticalSemanticAnalyzer::StaticType &&type,
                PracticalSemanticAnalyzer::ExpressionId lvalueId);
    };

    class Function {
    public:
        const Tokenizer::Token *name;
        PracticalSemanticAnalyzer::IdentifierId id;
        PracticalSemanticAnalyzer::StaticType returnType;
        std::vector<PracticalSemanticAnalyzer::ArgumentDeclaration> arguments;

        Function( const Tokenizer::Token *name );
    };

    using NamedObject = std::variant< LocalVariable, Function >;
    
private:
    static std::unordered_map<PracticalSemanticAnalyzer::TypeId, const NamedType *> typeRepository;

    const LookupContext *parent;

    std::unordered_map< String, NamedType > types;
    std::unordered_map< String, NamedObject > symbols;

public:
    LookupContext( const LookupContext *parent );
    ~LookupContext();

    const LookupContext *getParent() const { return parent; }

    PracticalSemanticAnalyzer::TypeId registerType( const Tokenizer::Token *name, NamedType::Type type, size_t size );
    Function *registerFunctionPass1( const Tokenizer::Token *name );
    void registerFunctionPass2(
            String name,
            PracticalSemanticAnalyzer::StaticType &&returnType,
            std::vector<PracticalSemanticAnalyzer::ArgumentDeclaration> &&arguments);
    const LocalVariable *registerVariable( LocalVariable &&variable );

    const NamedObject *lookupIdentifier(String name) const;
    const NamedType *lookupType(String name) const;

    static const PracticalSemanticAnalyzer::NamedType *lookupType(PracticalSemanticAnalyzer::TypeId id);
};

#endif // LOOKUP_CONTEXT_H
