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
    // Arithmetic operators: +, -, *, / and %.
    enum class ArithmeticOp : std::uint8_t
    {
        Add, // +
        Minus, // -
        Multiply, // *
        Divide, // /
        Modulo, // %
    };

    // Increment and decrement operators: ++ and --.
    enum class IncDecOp : std::uint8_t
    {
        PlusPlus, // ++
        MinusMinus, // --
    };

    // Simple assignment and compound assignment operators.
    enum class AssignOp : std::uint8_t
    {
        Assign, // =
        PlusAssign, // +=
        MinusAssign, // -=
        StarAssign, // *=
        SlashAssign, // /=
        PercentAssign, // %=

        AmpersandAssign, // &=
        PipeAssign, // |=
        CaretAssign, // ^=
        LeftShiftAssign, // <<=
        RightShiftAssign, // >>=
        UnsignedRightShiftAssign, // >>>=
    };

    // Equality and ordering operators.
    enum class CompareOp : std::uint8_t
    {
        Equal, // ==
        NotEqual, // !=
        Less, // <
        LessEqual, // <=
        Greater, // >
        GreaterEqual, // >=
    };

    // Short-circuit boolean operators.
    enum class LogicalOp : std::uint8_t
    {
        And, // &&
        Or, // ||
    };

    // Bitwise operators, also used for boolean operands.
    enum class BitwiseOp : std::uint8_t
    {
        And, // &
        Or, // |
        Xor, // ^
    };

    // Signed and unsigned bit-shift operators.
    enum class ShiftOp : std::uint8_t
    {
        Left, // <<
        Right, // >>
        UnsignedRight, // >>>
    };

    // Unary sign, boolean negation and bitwise complement.
    enum class UnaryOp : std::uint8_t
    {
        Plus, // +
        Minus, // -
        Not, // !
        Complement, // ~
    };

    // Common left and right operands of a binary expression.
    struct BinaryExpr : Expr
    {
        std::unique_ptr<Expr> left; // Left operand.
        std::unique_ptr<Expr> right; // Right operand.
    };

    // Arithmetic: left + right, left * right, etc.
    struct ArithmeticExpr : BinaryExpr
    {
        ArithmeticOp op = ArithmeticOp::Add; // Operation applied to the operand(s).
    };

    // Increment or decrement: ++x, x++, --x or x--.
    struct IncDecExpr : Expr
    {
        IncDecOp op = IncDecOp::PlusPlus; // Operation applied to the operand(s).
        std::unique_ptr<Expr> operand; // Expression being operated on.
        bool isPostfix = false; // True for x++; false for ++x (also applies to --).
    };

    // Assignment: left = right or left += right, etc.
    struct AssignExpr : BinaryExpr
    {
        AssignOp op = AssignOp::Assign; // Operation applied to the operand(s).
    };

    // Comparison: ==, !=, <, <=, > or >=.
    struct CompareExpr : BinaryExpr
    {
        CompareOp op = CompareOp::Equal; // Operation applied to the operand(s).
    };

    // Short-circuit boolean operation: && or ||.
    struct LogicalExpr : BinaryExpr
    {
        LogicalOp op = LogicalOp::And; // Operation applied to the operand(s).
    };

    // Bitwise or non-short-circuit boolean operation: &, | or ^.
    struct BitwiseExpr : BinaryExpr
    {
        BitwiseOp op = BitwiseOp::And; // Operation applied to the operand(s).
    };

    // Bit shift: <<, >> or >>>.
    struct ShiftExpr : BinaryExpr
    {
        ShiftOp op = ShiftOp::Left; // Operation applied to the operand(s).
    };

    // Unary operation: +x, -x, !x or ~x.
    struct UnaryExpr : Expr
    {
        UnaryOp op = UnaryOp::Plus; // Operation applied to the operand(s).
        std::unique_ptr<Expr> operand; // Expression being operated on.
    };

    // Conditional expression: condition ? whenTrue : whenFalse.
    struct ConditionalExpr : Expr
    {
        std::unique_ptr<Expr> condition; // Condition expression.
        std::unique_ptr<Expr> whenTrue; // Value selected when condition is true.
        std::unique_ptr<Expr> whenFalse; // Value selected when condition is false.
    };

    // An unresolved name: x or Foo.
    struct NameExpr : Expr
    {
        std::u16string name; // Declared or referenced name.
    };

    // A parenthesized expression: (a + b).
    struct ParenExpr : Expr
    {
        std::unique_ptr<Expr> expression; // Wrapped expression.
    };

    // this or a qualified Outer.this.
    struct ThisExpr : Expr
    {
        std::u16string qualifier; // Outer in Outer.this/super; empty when unqualified.
    };

    // super or a qualified Outer.super.
    struct SuperExpr : Expr
    {
        std::u16string qualifier; // Outer in Outer.this/super; empty when unqualified.
    };

    // Member access: receiver.name.
    struct FieldAccessExpr : Expr
    {
        std::unique_ptr<Expr> receiver; // Expression before the member-access dot.
        std::u16string name; // Declared or referenced name.
    };

    // Array access: arrayExpr[indexExpr].
    struct ArrayAccessExpr : Expr
    {
        std::unique_ptr<Expr> arrayExpr; // Array being indexed.
        std::unique_ptr<Expr> indexExpr; // Array index expression.
    };

    // Method invocation: foo(1) or obj.<T>foo(1).
    struct CallExpr : Expr
    {
        std::unique_ptr<Expr> callee; // Expression naming the method, e.g. foo or obj.foo.
        std::vector<TypeNode> typeArguments; // Explicit type arguments at this use site.
        std::vector<std::unique_ptr<Expr>> arguments; // Arguments in source order.
    };

    // Class instance creation: new Foo(1), optionally with an anonymous body.
    struct NewExpr : Expr
    {
        std::unique_ptr<Expr> qualifier; // Enclosing instance in outer.new Inner(); null for plain new.
        std::vector<TypeNode> typeArguments; // Constructor type arguments in new <T> Foo(); class arguments are in type.
        TypeNode type; // Class type being instantiated.
        std::vector<std::unique_ptr<Expr>> arguments; // Arguments in source order.
        std::optional<std::vector<std::unique_ptr<Decl>>> members; // Absent without a class body; present and empty for {}.
    };

    // Array initializer: {1, 2} or {{1}, {2}}.
    struct ArrayInitializerExpr : Expr
    {
        std::vector<std::unique_ptr<Expr>> elements; // Element expressions; nested initializers are allowed.
    };

    // One new-array dimension: [sizeExpr] or [].
    struct ArrayDimensionExpr : ArrayDimension
    {
        std::unique_ptr<Expr> sizeExpr; // Dimension length; null for an unsized [].
    };

    // Array creation: new int[3][] or new int[]{1, 2}.
    struct NewArrayExpr : Expr
    {
        TypeNode elementType; // Base element type for an array; e.g. int in int[][].
        std::vector<ArrayDimensionExpr> dimensions; // Array dimensions in source order.
        std::unique_ptr<ArrayInitializerExpr> initializer; // Optional initializer; null when no initializer was written.
    };

    // A class literal: String.class, int[].class or void.class.
    struct ClassLiteralExpr : Expr
    {
        TypeNode type; // Type before .class.
    };

    // A cast: (int)value or (A & B)value.
    struct CastExpr : Expr
    {
        TypeNode castType; // First type in the cast.
        std::vector<TypeNode> additionalBounds; // Remaining types in an intersection cast: B in (A & B).
        std::unique_ptr<Expr> value; // Expression being converted.
    };

    // Type test or pattern match: value instanceof String [s].
    struct InstanceOfExpr : Expr
    {
        std::unique_ptr<Expr> value; // Expression being tested.
        std::optional<TypeNode> testType; // Plain instanceof type; absent when pattern is set.
        std::unique_ptr<Pattern> pattern; // Matching pattern; null for a plain type test.
    };

    // A lambda: x -> x + 1 or x -> { return x; }.
    struct LambdaExpr : Expr
    {
        std::vector<std::unique_ptr<ParameterDecl>> parameters; // Formal parameters in source order.
        std::unique_ptr<Expr> exprBody; // Expression lambda body; mutually exclusive with blockBody.
        std::unique_ptr<BlockStatement> blockBody; // Block lambda body; mutually exclusive with exprBody.
    };

    // Method or constructor reference: obj::run, Type::run or Type::new.
    struct MethodRefExpr : Expr
    {
        std::unique_ptr<Expr> exprQualifier; // Expression before ::; mutually exclusive with typeQualifier.
        std::optional<TypeNode> typeQualifier; // Type before ::; mutually exclusive with exprQualifier.
        std::vector<TypeNode> typeArguments; // Explicit type arguments at this use site.
        std::optional<std::u16string> methodName; // Method name; absent for ::new.
    };

    // A switch that produces a value: switch (x) { ... }.
    struct SwitchExpr : Expr
    {
        std::unique_ptr<Expr> selectorExpr; // Expression inside switch (...).
        std::vector<SwitchCase> cases; // Switch branches in source order.
    };

    // An int literal or constant: 123.
    struct IntExpr : Expr
    {
        jint value{}; // Constant value.
    };

    // A long literal or constant: 123L.
    struct LongExpr : Expr
    {
        jlong value{}; // Constant value.
    };

    // A float literal or constant: 1.5F.
    struct FloatExpr : Expr
    {
        jfloat value{}; // Constant value.
    };

    // A double literal or constant: 1.5D.
    struct DoubleExpr : Expr
    {
        jdouble value{}; // Constant value.
    };

    // A UTF-16 character literal or constant: 'a'.
    struct CharExpr : Expr
    {
        jchar value{}; // Constant value.
    };

    // A short constant; ordinary integer literals start as int or long.
    struct ShortExpr : Expr
    {
        jshort value{}; // Constant value.
    };

    // A byte constant; ordinary integer literals start as int or long.
    struct ByteExpr : Expr
    {
        jbyte value{}; // Constant value.
    };

    // A boolean literal or constant: true or false.
    struct BooleanExpr : Expr
    {
        jboolean value{}; // Constant value.
    };

    // A decoded string literal or text block.
    struct StringExpr : Expr
    {
        std::u16string value; // Decoded UTF-16 contents, without quotes.
        bool textBlock = false; // True for a triple-quoted text block.
    };

    // The null literal.
    struct NullExpr : Expr {};

} // javac

#endif //JAVACOMPILER_EXPRESSIONS_H
