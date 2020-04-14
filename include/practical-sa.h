/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
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

#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <variant>

// An unsigned int type long enough to be castable to any int type without losing precision
using LongEnoughInt = std::uintmax_t;
using LongEnoughIntSigned = std::intmax_t;

// Hopefully unique module ID for our typed ids
static constexpr size_t PracticalSAModuleId = 0xc5489da402dc5a84;
DECL_TYPED_NS( PracticalSemanticAnalyzer, ModuleId, unsigned long, 0, PracticalSAModuleId );
DECL_TYPED_NS( PracticalSemanticAnalyzer, IdentifierId, unsigned long, 0, PracticalSAModuleId );
DECL_TYPED_NS( PracticalSemanticAnalyzer, ExpressionId, unsigned long, 0, PracticalSAModuleId );
DECL_TYPED_NS( PracticalSemanticAnalyzer, JumpPointId, unsigned long, 0, PracticalSAModuleId );

namespace PracticalSemanticAnalyzer {
    class CompilerArguments {
    public:
    };

    // Cookie type used by the backend to identify types. Backend can choose whether to use an integer or a pointer
    union TypeId {
        uintptr_t n;
        void *p;
    };

    class StaticType : private NoCopy, public boost::intrusive_ref_counter<StaticType, boost::thread_unsafe_counter> {
    public:
        class Scalar {
            size_t size=0, alignment=1;
            TypeId typeId;

        public:
            Scalar( size_t size, size_t alignment, TypeId typeId ) : size(size), alignment(alignment), typeId(typeId) {}

            virtual String getName() const = 0;

            size_t getSize() const {
                return size;
            }

            size_t getAlignment() const {
                return alignment;
            }

            TypeId getTypeId()  const {
                return typeId;
            }
        };

        using CPtr = boost::intrusive_ptr<const StaticType>;
        using Types = std::variant<const Scalar *>;

        virtual ~StaticType() {}

        virtual Types getType() const = 0;
    };
    std::ostream &operator<<(std::ostream &out, StaticType::CPtr type);

    class BuiltinContextGen {
    public:
        virtual TypeId registerVoidType( String name ) = 0;
        virtual TypeId registerBoolType( String name ) = 0;
        virtual TypeId registerIntegerType( String name, size_t bitSize, size_t alignment, bool _signed ) = 0;
        virtual TypeId registerCharType( String name, size_t bitSize, size_t alignment, bool _signed ) = 0;
    };

    struct ArgumentDeclaration {
        StaticType::CPtr type;
        String name;
        ExpressionId lvalueId;

        ArgumentDeclaration(StaticType::CPtr _type, String _name, ExpressionId _lvalueId)
                : type(_type), name(_name), lvalueId(_lvalueId)
        {}
    };

    /// Callbacks used by the semantic analyzer to allow the SA user to actually generate code
    class FunctionGen {
    public:
        // Function handling
        virtual void functionEnter(
                IdentifierId id, String name, StaticType::CPtr returnType, Slice<const ArgumentDeclaration> arguments,
                String file, size_t line, size_t col) = 0;
        virtual void functionLeave(IdentifierId id) = 0;

        virtual void returnValue(ExpressionId id) = 0;
        virtual void returnValue() = 0; // For Void functions

        // Flow control

        // If id==ExpressionId(), then the code gen SHOULD assume that the result of the expression is never used.
        // If elsePoint==JumpPointId(), there is no else clause.
        // practical-sa will generate the jump points, but will not generate the jumps. It is up to the code generation
        // to use the jump points to identify the code flow.
        virtual void conditionalBranch(
                ExpressionId id, StaticType::CPtr type, ExpressionId conditionExpression, JumpPointId elsePoint,
                JumpPointId continuationPoint
            ) = 0;
        // Called twice, once for "then" and once for "else", to signify which expression is that clause's return
        virtual void setConditionClauseResult( ExpressionId id ) = 0;
        virtual void setJumpPoint(JumpPointId id, String name = String()) = 0;
        virtual void jump(JumpPointId destination) = 0;

        // Litarals
        virtual void setLiteral(ExpressionId id, LongEnoughInt value, StaticType::CPtr type) = 0;
        virtual void setLiteral(ExpressionId id, bool value) = 0;

        // The ExpressionId refers to a pointer to the resulting allocated variable
        virtual void allocateStackVar(ExpressionId id, StaticType::CPtr type, String name) = 0;
        virtual void assign( ExpressionId lvalue, ExpressionId rvalue ) = 0;
        virtual void dereferencePointer( ExpressionId id, StaticType::CPtr type, ExpressionId addr ) = 0;

        // Casts
        virtual void truncateInteger(
                ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType ) = 0;
        virtual void expandIntegerSigned(
                ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType ) = 0;
        virtual void expandIntegerUnsigned(
                ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType ) = 0;
        virtual void callFunctionDirect(
                ExpressionId id, String name, Slice<const ExpressionId> arguments, StaticType::CPtr returnType ) = 0;

        // Binary operators
        virtual void binaryOperatorPlus(
                ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) = 0;
        virtual void binaryOperatorMinus(
                ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) = 0;
    };

    class ModuleGen {
    public:
        virtual void moduleEnter(ModuleId id, String name, String file, size_t line, size_t col) = 0;
        virtual void moduleLeave(ModuleId id) = 0;

        virtual std::shared_ptr<FunctionGen> handleFunction(IdentifierId id) = 0;
    };

    std::unique_ptr<CompilerArguments> allocateArguments();

    // Must be called exactly once, before starting actual compilation
    void prepare( BuiltinContextGen *ctxGen ); // This is the lookup context used for the builtin types
    // XXX Should path actually be a buffer?
    int compile(std::string path, const CompilerArguments *arguments, ModuleGen *codeGen);
} // End namespace PracticalSemanticAnalyzer

#endif // LIB_PRACTICAL_SA_H
