//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_EXPRESSIONS_H
#define JAVACOMPILER_EXPRESSIONS_H
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Declarations.h"
#include "Statements.h"
#include "main/ast/ASTDef.h"

namespace javac
{
    enum class ArthOp : std::uint8_t
    {
        Add,            // +
        Minus,          // -
        Multiply,       // *
        Divide,         // /
        Modulo,         // %
    };

    enum class ArthExtraOp : std::uint8_t
    {
        PlusPlus,       // ++
        MinusMinus,     // --
    };

    enum class AssignOp : std::uint8_t
    {
        Assign,                    // =
        PlusAssign,                // +=
        MinusAssign,               // -=
        StarAssign,                // *=
        SlashAssign,               // /=
        PercentAssign,             // %=

        AmpersandAssign,           // &=
        PipeAssign,                // |=
        CaretAssign,               // ^=
        LeftShiftAssign,           // <<=
        RightShiftAssign,          // >>=
        UnsignedRightShiftAssign,  // >>>=
    };

    enum class CompareOp : std::uint8_t
    {
        Equal,          // ==
        NotEqual,       // !=
        Less,           // <
        LessEqual,      // <=
        Greater,        // >
        GreaterEqual,   // >=
    };

    enum class LogicalOp : std::uint8_t
    {
        And,            // &&
        Or,             // ||
    };

    enum class BitwiseOp : std::uint8_t
    {
        And,            // &
        Or,             // |
        Xor,            // ^
    };

    enum class ShiftOp : std::uint8_t
    {
        Left,           // <<
        Right,          // >>
        UnsignedRight,  // >>>
    };

    enum class UnaryOp : std::uint8_t
    {
        Plus,           // +
        Minus,          // -
        Not,            // !
        Complement,     // ~
    };

    struct BinaryExpr : Expr
    {
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;
    };

    // left + right, left % right
    struct ArthExpr : BinaryExpr
    {
        ArthOp op = ArthOp::Add;
    };

    // ++operand, operand--
    struct ArthExtraExpr : Expr
    {
        ArthExtraOp op = ArthExtraOp::PlusPlus;
        std::unique_ptr<Expr> operand;
        bool postfix = false;
    };

    // left = right, left += right
    struct AssignExpr : BinaryExpr
    {
        AssignOp op = AssignOp::Assign;
    };

    struct CompareExpr : BinaryExpr
    {
        CompareOp op = CompareOp::Equal;
    };

    struct LogicalExpr : BinaryExpr
    {
        LogicalOp op = LogicalOp::And;
    };

    struct BitwiseExpr : BinaryExpr
    {
        BitwiseOp op = BitwiseOp::And;
    };

    struct ShiftExpr : BinaryExpr
    {
        ShiftOp op = ShiftOp::Left;
    };

    struct UnaryExpr : Expr
    {
        UnaryOp op = UnaryOp::Plus;
        std::unique_ptr<Expr> operand;
    };

    struct ConditionalExpr : Expr
    {
        std::unique_ptr<Expr> condition;
        std::unique_ptr<Expr> whenTrue;
        std::unique_ptr<Expr> whenFalse;
    };

    struct NameExpr : Expr
    {
        std::u16string name;
    };

    struct ParenthesizedExpr : Expr
    {
        std::unique_ptr<Expr> expression;
    };

    struct ThisExpr : Expr
    {
        std::u16string qualifier;
    };

    struct SuperExpr : Expr
    {
        std::u16string qualifier;
    };

    struct FieldAccessExpr : Expr
    {
        std::unique_ptr<Expr> receiver;
        std::u16string name;
    };

    struct ArrayAccessExpr : Expr
    {
        std::unique_ptr<Expr> array;
        std::unique_ptr<Expr> index;
    };

    struct CallExpr : Expr
    {
        std::unique_ptr<Expr> callee;
        std::vector<TypeExpr> typeArguments;
        std::vector<std::unique_ptr<Expr>> arguments;
    };

    struct NewClassExpr : Expr
    {
        std::unique_ptr<Expr> qualifier;
        std::vector<TypeExpr> typeArguments; // Explicit constructor type arguments
        TypeExpr type;
        std::vector<std::unique_ptr<Expr>> arguments;
        std::optional<std::vector<std::unique_ptr<Decl>>> members;
    };

    struct ArrayInitializerExpr : Expr
    {
        std::vector<std::unique_ptr<Expr>> elements;
    };

    struct ArrayDimensionExpr : ArrayDimension
    {
        std::unique_ptr<Expr> size;
    };

    struct NewArrayExpr : Expr
    {
        TypeExpr elementType;
        std::vector<ArrayDimensionExpr> dimensions;
        std::unique_ptr<ArrayInitializerExpr> initializer;
    };

    struct ClassLiteralExpr : Expr
    {
        TypeExpr type;
    };

    // Cast a value to type: (int)1.24D
    struct CastExpr : Expr
    {
        TypeExpr castType;
        std::vector<TypeExpr> additionalBounds; // (A & B)
        std::unique_ptr<Expr> value;
    };

    struct InstanceOfExpr : Expr
    {
        std::unique_ptr<Expr> value;
        std::optional<TypeExpr> type;
        std::unique_ptr<Pattern> pattern;
    };

    struct LambdaExpr : Expr
    {
        std::vector<std::unique_ptr<ParameterDecl>> parameters;
        std::unique_ptr<Expr> expressionBody;
        std::unique_ptr<BlockStatement> blockBody;
    };

    struct MethodReferenceExpr : Expr
    {
        std::unique_ptr<Expr> expressionQualifier;
        std::optional<TypeExpr> typeQualifier;
        std::vector<TypeExpr> typeArguments;
        std::optional<std::u16string> name; // Empty for ::new
    };

    struct SwitchExpr : Expr
    {
        std::unique_ptr<Expr> selector;
        std::vector<SwitchCase> cases;
    };

    // Literal and constant expressions
    struct IntExpr : Expr
    {
        jint value{};           // 1234
    };

    struct LongExpr : Expr
    {
        jlong value{};          // 1234L
    };

    struct FloatExpr : Expr
    {
        jfloat value{};         // 12.34F
    };

    struct DoubleExpr : Expr
    {
        jdouble value{};        // 12.34D
    };

    struct CharExpr : Expr
    {
        jchar value{};          // 'c'
    };

    struct ShortExpr : Expr
    {
        jshort value{};
    };

    struct ByteExpr : Expr
    {
        jbyte value{};
    };

    struct BooleanExpr : Expr
    {
        jboolean value{};       // true, false
    };

    struct StringExpr : Expr
    {
        std::u16string value;   // "Hello world!"
        bool textBlock = false;
    };

    struct NullExpr : Expr {};

} // javac

#endif //JAVACOMPILER_EXPRESSIONS_H
