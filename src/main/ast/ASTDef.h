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
    struct ASTNode
    {
        std::optional<SourceRange> range;

        virtual ~ASTNode() = default;
    };

    struct Expr : ASTNode {};
    struct Statement : ASTNode {};
    struct Decl : ASTNode {};
    struct Pattern : ASTNode {};

    struct AnnotationArg : ASTNode
    {
        std::optional<std::u16string> name; // Empty for @Name(value)
        std::unique_ptr<Expr> value;
    };

    struct Annotation : Expr
    {
        std::u16string name;
        std::vector<AnnotationArg> arguments;
    };

    struct AdditionalFlags : ASTNode
    {
        std::optional<FlagType> type;
        std::optional<Annotation> annotation;
    };

    enum class ObjectType : std::uint8_t
    {
        Void,           // void
        Primitive,      // boolean, byte, short, char, int, long, float, double
        Class,          // Object, String, Boolean
        Array,          // int[], String[], byte[]
        GenericClass,   // List<String>, Function<Boolean>
        Inferred,       // var
    };

    enum class GenericType : std::uint8_t
    {
        Exact,          // String
        Unbounded,      // ?
        Extends,        // ? extends Number
        Super,          // ? super Integer
    };

    struct TypeExpr;

    struct TypeArg : ASTNode
    {
        GenericType genericType = GenericType::Exact;
        std::vector<Annotation> annotations; // Annotations on ?
        std::unique_ptr<TypeExpr> type;       // Exact type or wildcard bound
    };

    struct TypeNamePart : ASTNode
    {
        std::u16string name;
        std::vector<Annotation> annotations;
        std::vector<TypeArg> typeArguments;
        bool diamond = false; // <>
    };

    struct ArrayDimension : ASTNode
    {
        std::vector<Annotation> annotations;
    };

    struct TypeExpr : ASTNode
    {
        ObjectType type = ObjectType::Class;
        std::vector<Annotation> annotations; // Annotations on a primitive, void or var
        std::optional<PrimitiveType> primitive;
        std::vector<TypeNamePart> name;      // Each part of Outer<T>.Inner<U>
        std::unique_ptr<TypeExpr> elementType;
        std::vector<ArrayDimension> dimensions;
    };

    struct TypeParameter : ASTNode
    {
        std::vector<Annotation> annotations;
        std::u16string name;
        std::vector<TypeExpr> bounds; // T extends A & B
    };

    struct VariableDeclarator : ASTNode
    {
        std::u16string name;
        std::vector<ArrayDimension> dimensions; // Dimensions after the name
        std::unique_ptr<Expr> initializer;
    };

    struct VariableDecl : Decl
    {
        std::vector<AdditionalFlags> modifiers;
        TypeExpr type;
        std::vector<VariableDeclarator> declarators;
    };

    struct TypePattern : Pattern
    {
        std::vector<AdditionalFlags> modifiers;
        TypeExpr type;
        std::u16string name;
    };

    struct RecordPattern : Pattern
    {
        TypeExpr type;
        std::vector<std::unique_ptr<Pattern>> components;
    };

    struct UnnamedPattern : Pattern {};

} // javac

#endif //JAVACOMPILER_ASTDEF_H
