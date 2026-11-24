//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_ASTDEF_H
#define JAVACOMPILER_ASTDEF_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "main/core/DataDef.h"
#include "main/core/def/JavaDef.h"

namespace javac
{
    // Base of all AST nodes; children are owned by their parent.
    struct ASTNode
    {
        std::optional<SourceRange> range; // Source span [begin, end); absent for generated nodes.

        virtual ~ASTNode() = default;
    };

    // An expression, such as a + b or foo().
    struct Expr : ASTNode {};
    // An executable statement, such as return x; or if (...).
    struct Statement : ASTNode {};
    // A declaration, such as a class, method or variable.
    struct Decl : ASTNode {};
    // A matching pattern used by instanceof or switch.
    struct Pattern : ASTNode {};

    // One annotation argument: value = 1 or just 1.
    struct AnnotationArg : ASTNode
    {
        std::optional<std::u16string> name; // Argument name; absent for the single-value form @Name(1).
        std::unique_ptr<Expr> value; // Annotation argument expression.
    };

    // An annotation use: @Name or @Name(value = 1).
    struct Annotation : Expr
    {
        std::u16string name; // Declared or referenced name.
        std::vector<AnnotationArg> arguments; // Arguments in source order.
    };

    // One keyword modifier or annotation; set exactly one of flagType and annotation.
    struct ModifierFlag : ASTNode
    {
        std::optional<FlagType> flagType; // Keyword modifier, such as public or static; absent for an annotation.
        std::optional<Annotation> annotation; // Annotation modifier; absent for a keyword modifier.
    };

    // The form of a written type; not a resolved runtime type.
    enum class ObjectType : std::uint8_t
    {
        Void, // void
        Primitive, // boolean, byte, short, char, int, long, float, double
        Class, // Object, String, Boolean
        Array, // int[], String[], byte[]
        GenericClass, // List<String>, Function<Boolean>
        Inferred, // var
    };

    // The form of a type argument: T, ?, ? extends T or ? super T.
    enum class GenericType : std::uint8_t
    {
        Exact, // String
        Unbounded, // ?
        Extends, // ? extends Number
        Super, // ? super Integer
    };

    struct TypeNode;

    // One type argument inside List<T> or List<? extends T>.
    struct TypeArg : ASTNode
    {
        GenericType genericType = GenericType::Exact; // Exact type or wildcard form.
        std::vector<Annotation> annotations; // Annotations on the wildcard ?.
        std::unique_ptr<TypeNode> type; // Exact argument or wildcard bound; null for unbounded ?.
    };

    // One name segment: Outer<T> or Inner<U> in Outer<T>.Inner<U>.
    struct TypeNamePart : ASTNode
    {
        std::u16string name; // Declared or referenced name.
        std::vector<Annotation> annotations; // Annotations attached to this syntax element.
        std::vector<TypeArg> typeArguments; // Explicit type arguments at this use site.
        bool diamond = false; // True for <>; typeArguments is empty in that form.
    };

    // One [] in a type, including annotations on that dimension.
    struct ArrayDimension : ASTNode
    {
        std::vector<Annotation> annotations; // Annotations attached to this syntax element.
    };

    // A written type; fill the fields matching objectType.
    struct TypeNode : ASTNode
    {
        ObjectType objectType = ObjectType::Class; // Selects which type fields below are meaningful.
        std::vector<Annotation> annotations; // Annotations on a primitive, void or var; class annotations belong to nameParts.
        std::optional<PrimitiveType> primitiveType; // Set only for ObjectType::Primitive, such as int or boolean.
        std::vector<TypeNamePart> nameParts; // Qualified class name segments, each with its own type arguments.
        std::unique_ptr<TypeNode> elementType; // Base element type for an array; e.g. int in int[][].
        std::vector<ArrayDimension> dimensions; // Array dimensions in source order.
    };

    // A declared type parameter: T extends A & B.
    struct GenericParameter : ASTNode
    {
        std::vector<Annotation> annotations; // Annotations attached to this syntax element.
        std::u16string name; // Declared or referenced name.
        std::vector<TypeNode> bounds; // Upper bounds: A and B in T extends A & B.
    };

    // One variable in a declaration: a = 1 or b[] in int a = 1, b[].
    struct VariableDeclarator : ASTNode
    {
        std::u16string name; // Declared or referenced name.
        std::vector<ArrayDimension> dimensions; // Extra [] after the variable name: int value[].
        std::unique_ptr<Expr> initializer; // Optional initializer; null when no initializer was written.
    };

    // Shared type and modifiers for one or more declared variables.
    struct VariableDecl : Decl
    {
        std::vector<ModifierFlag> modifiers; // Keyword modifiers and annotations in source order.
        TypeNode type; // Shared written type of all declarators.
        std::vector<VariableDeclarator> declarators; // Variables sharing the declared type and modifiers.
    };

    // Match a type and bind a name: String s.
    struct TypePattern : Pattern
    {
        std::vector<ModifierFlag> modifiers; // Keyword modifiers and annotations in source order.
        TypeNode type; // Type to match.
        std::u16string name; // Declared or referenced name.
    };

    // Match record components recursively: Point(int x, int y).
    struct RecordPattern : Pattern
    {
        TypeNode type; // Record type to deconstruct.
        std::vector<std::unique_ptr<Pattern>> components; // Components in source order.
    };

    // An unnamed pattern: _.
    struct UnnamedPattern : Pattern {};

} // javac

#endif //JAVACOMPILER_ASTDEF_H
