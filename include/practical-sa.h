/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef LIB_PRACTICAL_SA_H
#define LIB_PRACTICAL_SA_H

#include "nocopy.h"
#include "typed.h"
#include "slice.h"

#include <iostream>
#include <memory>
#include <string>
#include <variant>

// An unsigned int type long enough to be castable to any int type without losing precision
typedef unsigned long long LongEnoughInt;

namespace PracticalSemanticAnalyzer {
    // Hopefully unique module ID for our typed ids
    static constexpr size_t PracticalSAModuleId = 0xc5489da402dc5a84;

    typedef Typed<unsigned long, 0, PracticalSAModuleId, __LINE__> ModuleId;
    typedef Typed<unsigned long, 0, PracticalSAModuleId, __LINE__> IdentifierId;
    typedef Typed<unsigned long, 0, PracticalSAModuleId, __LINE__> TypeId;
    typedef Typed<unsigned long, 0, PracticalSAModuleId, __LINE__> ExpressionId;

    class CompilerArguments {
    public:
    };

    class StaticType : private NoCopy {
    private:
        TypeId id;

    public:
        StaticType() = default;
        StaticType(TypeId _id) : id(_id) {
        }
        StaticType(StaticType &&that) : id(that.id) {
            that.id = TypeId(0);
        }

        StaticType &operator=( StaticType &&that );

        TypeId getId() const {
            return id;
        }

        void setId( TypeId id ) {
            this->id = id;
        }

        bool operator==( const StaticType &that ) const {
            return this->id==that.id;
        }

        bool operator!=( const StaticType &that ) const {
            return !operator==( that );
        }
    };
    std::ostream &operator<<(std::ostream &out, const StaticType &type);

    struct ArgumentDeclaration {
        const StaticType &type;
        String name;
        ExpressionId lvalueId;

        ArgumentDeclaration(const StaticType &_type, String _name, ExpressionId _lvalueId)
                : type(_type), name(_name), lvalueId(_lvalueId)
        {}
    };

    /// Callbacks used by the semantic analyzer to allow the SA user to actually generate code
    class FunctionGen {
    public:
        virtual void functionEnter(
                IdentifierId id, String name, const StaticType& returnType, Slice<const ArgumentDeclaration> arguments,
                String file, size_t line, size_t col) = 0;
        virtual void functionLeave(IdentifierId id) = 0;

        virtual void returnValue(ExpressionId id) = 0;
        virtual void setLiteral(ExpressionId id, LongEnoughInt value, const StaticType &type) = 0;

        // The ExpressionId refers to a pointer to the resulting allocated variable
        virtual void allocateStackVar(ExpressionId id, const StaticType &type, String name) = 0;
        virtual void assign( ExpressionId lvalue, ExpressionId rvalue ) = 0;
        virtual void dereferencePointer( ExpressionId id, const StaticType &type, ExpressionId addr ) = 0;

        virtual void truncateInteger(
                ExpressionId id, ExpressionId source, const StaticType &sourceType, const StaticType &destType ) = 0;
        virtual void expandIntegerSigned(
                ExpressionId id, ExpressionId source, const StaticType &sourceType, const StaticType &destType ) = 0;
        virtual void expandIntegerUnsigned(
                ExpressionId id, ExpressionId source, const StaticType &sourceType, const StaticType &destType ) = 0;
        virtual void callFunctionDirect(
                ExpressionId id, String name, Slice<const ExpressionId> arguments, const StaticType &returnType ) = 0;
    };

    class ModuleGen {
    public:
        virtual void moduleEnter(ModuleId id, String name, String file, size_t line, size_t col) = 0;
        virtual void moduleLeave(ModuleId id) = 0;

        virtual std::shared_ptr<FunctionGen> handleFunction(IdentifierId id) = 0;
    };

    std::unique_ptr<CompilerArguments> allocateArguments();

    // XXX Should path actually be a buffer?
    int compile(std::string path, const CompilerArguments *arguments, ModuleGen *codeGen);

    class NamedType {
    public:
        enum class Type { Void, Char, UnsignedInteger, SignedInteger, Boolean };

        virtual size_t size() const = 0;
        virtual String name() const = 0;
        virtual Type type() const = 0;
        virtual TypeId id() const = 0;

        bool isBuiltin() const {
            return static_cast<int>( type() ) <= static_cast<int>( Type::Boolean );
        }
    };

    const NamedType *lookupTypeId(TypeId id);
} // End namespace PracticalSemanticAnalyzer

#endif // LIB_PRACTICAL_SA_H
