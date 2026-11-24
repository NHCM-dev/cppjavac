//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_DECLARATIONS_H
#define JAVACOMPILER_DECLARATIONS_H
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Statements.h"
#include "main/ast/ASTDef.h"

namespace javac
{
    struct PackageDecl : Decl
    {
        std::vector<Annotation> annotations;
        std::u16string name;
    };

    enum class ImportKind : std::uint8_t
    {
        Type,
        TypeOnDemand,
        Static,
        StaticOnDemand,
        Module,
    };

    struct ImportDecl : Decl
    {
        ImportKind kind = ImportKind::Type;
        std::u16string name;
    };

    enum class ModuleDirectiveKind : std::uint8_t
    {
        Requires,
        Exports,
        Opens,
        Uses,
        Provides,
    };

    struct ModuleDirective : ASTNode
    {
        ModuleDirectiveKind kind = ModuleDirectiveKind::Requires;
        std::u16string name;
        std::vector<std::u16string> targets; // Names after to or with
        bool isStatic = false;             // requires static
        bool transitive = false;           // requires transitive
    };

    struct ModuleDecl : Decl
    {
        std::vector<Annotation> annotations;
        std::u16string name;
        bool open = false;
        std::vector<ModuleDirective> directives;
    };

    enum class CompilationUnitKind : std::uint8_t
    {
        Ordinary,
        Modular,
        Compact,
    };

    struct CompilationUnit : ASTNode
    {
        CompilationUnitKind kind = CompilationUnitKind::Ordinary;
        std::optional<PackageDecl> package;
        std::vector<ImportDecl> imports;
        std::vector<std::unique_ptr<Decl>> declarations;
        std::optional<ModuleDecl> module;
    };

    struct EmptyDecl : Decl {};

    struct FieldDecl : VariableDecl {};

    struct ParameterDecl : Decl
    {
        std::vector<AdditionalFlags> modifiers;
        std::u16string name;
        std::optional<TypeExpr> type; // Empty for an implicitly typed lambda parameter
        std::vector<ArrayDimension> dimensions;
        std::optional<ArrayDimension> varargs; // Annotations on ...
        std::unique_ptr<Expr> defaultValue;    // AllowDefaultParameters
    };

    struct ReceiverParameter : ASTNode
    {
        TypeExpr type;
        std::optional<std::u16string> qualifier; // Outer in Outer.this
    };

    struct MethodDecl : Decl
    {
        std::vector<AdditionalFlags> modifiers;
        std::u16string name;

        std::vector<TypeParameter> typeParameters;
        TypeExpr returnType;
        std::optional<ReceiverParameter> receiver;

        std::vector<std::unique_ptr<ParameterDecl>> parameters;
        std::vector<ArrayDimension> returnDimensions; // Dimensions after the parameter list
        std::vector<TypeExpr> thrownTypes;
        std::unique_ptr<Expr> defaultValue; // Annotation element default

        std::unique_ptr<BlockStatement> body; // Empty for a declaration ending in ;
    };

    struct ConstructorDecl : Decl
    {
        std::vector<AdditionalFlags> modifiers;
        std::u16string name;

        bool compact = false;
        std::vector<TypeParameter> typeParameters;
        std::optional<ReceiverParameter> receiver;
        std::vector<std::unique_ptr<ParameterDecl>> parameters;
        std::vector<TypeExpr> thrownTypes;
        std::unique_ptr<BlockStatement> body;
    };

    struct InitializerDecl : Decl
    {
        bool isStatic = false;
        std::unique_ptr<BlockStatement> body;
    };

    struct TypeDecl : Decl
    {
        std::vector<AdditionalFlags> modifiers;
        std::u16string name;

        std::vector<std::unique_ptr<Decl>> members;
    };

    struct ClassDecl : TypeDecl
    {
        std::vector<TypeParameter> typeParameters;
        std::optional<TypeExpr> superClass;
        std::vector<TypeExpr> interfaces;
        std::vector<TypeExpr> permittedTypes;
    };

    struct InterfaceDecl : TypeDecl
    {
        std::vector<TypeParameter> typeParameters;
        std::vector<TypeExpr> superInterfaces;
        std::vector<TypeExpr> permittedTypes;
    };

    struct AnnotationDecl : TypeDecl {};

    struct EnumConstantDecl : Decl
    {
        std::vector<Annotation> annotations;
        std::u16string name;
        std::vector<std::unique_ptr<Expr>> arguments;
        std::optional<std::vector<std::unique_ptr<Decl>>> members;
    };

    struct EnumDecl : TypeDecl
    {
        std::vector<EnumConstantDecl> constants;
        std::vector<TypeExpr> interfaces;
    };

    struct RecordComponent : ASTNode
    {
        std::vector<Annotation> annotations;
        std::u16string name;
        TypeExpr type;
        std::optional<ArrayDimension> varargs;
    };

    struct RecordDecl : TypeDecl
    {
        std::vector<TypeParameter> typeParameters;
        std::vector<RecordComponent> components;
        std::vector<TypeExpr> interfaces;
    };
} // javac

#endif //JAVACOMPILER_DECLARATIONS_H
