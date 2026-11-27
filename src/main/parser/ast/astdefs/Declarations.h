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
#include "main/parser/ast/ASTDef.h"

namespace javac
{
    // Package declaration: package com.example;
    struct PackageDecl : Decl
    {
        std::vector<Annotation> annotations; // Annotations attached to this syntax element.
        std::u16string name; // Declared or referenced name.
    };

    // The form of an import declaration.
    enum class ImportType : std::uint8_t
    {
        Type, // import java.util.List;
        TypeOnDemand, // import java.util.*;
        Static, // import static java.lang.Math.PI;
        StaticOnDemand, // import static java.lang.Math.*;
        Module, // import module java.base;
    };

    // Import declaration; the name excludes import, static and the trailing .*.
    struct ImportDecl : Decl
    {
        ImportType importType = ImportType::Type; // Import form, including static and on-demand variants.
        std::u16string name; // Declared or referenced name.
    };

    // The keyword starting a module directive.
    enum class ModuleDirectiveType : std::uint8_t
    {
        Requires, // requires [static] [transitive] module.name;
        Exports, // exports package.name [to module.name];
        Opens, // opens package.name [to module.name];
        Uses, // uses ServiceType;
        Provides, // provides ServiceType with ImplementationType;
    };

    // One module directive: requires, exports, opens, uses or provides.
    struct ModuleDirective : ASTNode
    {
        ModuleDirectiveType directiveType = ModuleDirectiveType::Requires; // The directive keyword; determines how name and targetNames are used.
        std::u16string name; // Declared or referenced name.
        std::vector<std::u16string> targetNames; // Module names after to, or implementation types after with.
        bool isStatic = false; // True for requires static.
        bool isTransitive = false; // True for requires transitive.
    };

    // A module-info.java declaration: [open] module name { ... }.
    struct ModuleDecl : Decl
    {
        std::vector<Annotation> annotations; // Annotations attached to this syntax element.
        std::u16string name; // Declared or referenced name.
        bool isOpen = false; // True for open module.
        std::vector<ModuleDirective> directives; // Module directives in source order.
    };

    // Ordinary source, module-info.java or a compact source file.
    enum class CompilationUnitType : std::uint8_t
    {
        Ordinary, // A source file with top-level type declarations.
        Modular, // A module-info.java file.
        Compact, // A compact source file with an implicitly declared class.
    };

    // Root of one source file; moduleDecl is used for the Modular form.
    struct CompilationUnit : ASTNode
    {
        CompilationUnitType unitType = CompilationUnitType::Ordinary; // Selects ordinary, modular or compact source form.
        std::optional<PackageDecl> packageDecl; // Optional package declaration.
        std::vector<ImportDecl> importDecls; // Imports in source order.
        std::vector<Uptr<Decl>> decls; // Top-level declarations; compact form may also contain members.
        std::optional<ModuleDecl> moduleDecl; // Module declaration for a modular compilation unit.
    };

    // An empty declaration: a standalone ; in a declaration list.
    struct EmptyDecl : Decl {};

    // A field declaration inside a type: int x = 1;
    struct FieldDecl : VariableDecl {};

    // A method, constructor or lambda parameter.
    struct ParameterDecl : Decl
    {
        std::vector<ModifierFlag> modifiers; // Keyword modifiers and annotations in source order.
        std::u16string name; // Declared or referenced name.
        std::optional<TypeNode> type; // Absent for an implicitly typed lambda parameter: x -> x.
        std::vector<ArrayDimension> dimensions; // Extra [] after the variable name: int value[].
        std::optional<ArrayDimension> varargs; // Present for ...; stores annotations on the ellipsis.
        Uptr<Expr> defaultValue; // Optional parameter default; requires AllowDefaultParameters.
    };

    // An explicit receiver: A this or Outer Outer.this.
    struct ReceiverParameter : ASTNode
    {
        TypeNode type; // Explicit receiver type.
        std::optional<std::u16string> qualifier; // Outer in Outer.this; absent for plain this.
    };

    // A method or annotation element declaration.
    struct MethodDecl : Decl
    {
        std::vector<ModifierFlag> modifiers; // Keyword modifiers and annotations in source order.
        std::u16string name; // Declared or referenced name.

        std::vector<GenericParameter> typeParameters; // Declared generic parameters: <T extends A>.
        TypeNode returnType; // Written return type, including void.
        std::optional<ReceiverParameter> receiverParam; // Optional explicit this parameter; separate from ordinary parameters.

        std::vector<Uptr<ParameterDecl>> parameters; // Formal parameters in source order.
        std::vector<ArrayDimension> returnDimensions; // Extra [] after the parameter list: int method()[].
        std::vector<TypeNode> thrownTypes; // Exception types after throws.
        Uptr<Expr> defaultValue; // Annotation element default: int value() default 1;

        Uptr<BlockStatement> body; // Method body; null for a declaration ending in ;
    };

    // A constructor; compact form is used by records.
    struct ConstructorDecl : Decl
    {
        std::vector<ModifierFlag> modifiers; // Keyword modifiers and annotations in source order.
        std::u16string name; // Declared or referenced name.

        bool isCompact = false; // True for a record compact constructor: Point { ... }.
        std::vector<GenericParameter> typeParameters; // Declared generic parameters: <T extends A>.
        std::optional<ReceiverParameter> receiverParam; // Optional explicit this parameter; separate from ordinary parameters.
        std::vector<Uptr<ParameterDecl>> parameters; // Formal parameters in source order.
        std::vector<TypeNode> thrownTypes; // Exception types after throws.
        Uptr<BlockStatement> body; // Body of this construct.
    };

    // An instance { ... } or static { ... } initializer.
    struct InitializerDecl : Decl
    {
        bool isStatic = false; // True for the static form.
        Uptr<BlockStatement> body; // Body of this construct.
    };

    // Common name, modifiers and members of a declared type.
    struct TypeDecl : Decl
    {
        std::vector<ModifierFlag> modifiers; // Keyword modifiers and annotations in source order.
        std::u16string name; // Declared or referenced name.

        std::vector<Uptr<Decl>> members; // Member declarations in source order.
    };

    // class C<T> extends B implements I permits D { ... }.
    struct ClassDecl : TypeDecl
    {
        std::vector<GenericParameter> typeParameters; // Declared generic parameters: <T extends A>.
        std::optional<TypeNode> superClass; // Optional type after extends.
        std::vector<TypeNode> interfaces; // Types after implements.
        std::vector<TypeNode> permittedTypes; // Types in the permits clause of a sealed class or interface.
    };

    // interface I<T> extends A, B permits C { ... }.
    struct InterfaceDecl : TypeDecl
    {
        std::vector<GenericParameter> typeParameters; // Declared generic parameters: <T extends A>.
        std::vector<TypeNode> superInterfaces; // Types after extends.
        std::vector<TypeNode> permittedTypes; // Types in the permits clause of a sealed class or interface.
    };

    // An annotation type: @interface Name { ... }.
    struct AnnotationDecl : TypeDecl {};

    // One enum constant: RED(1) or RED { ... }.
    struct EnumConstantDecl : Decl
    {
        std::vector<Annotation> annotations; // Annotations attached to this syntax element.
        std::u16string name; // Declared or referenced name.
        std::vector<Uptr<Expr>> constructorArgs; // Arguments passed to the enum constructor.
        std::optional<std::vector<Uptr<Decl>>> members; // Absent without a class body; present and empty for {}.
    };

    // enum Color implements I { RED, BLUE; ... }.
    struct EnumDecl : TypeDecl
    {
        std::vector<EnumConstantDecl> constants; // Enum constants in source order.
        std::vector<TypeNode> interfaces; // Types after implements.
    };

    // One record header component: int x in record Point(int x).
    struct RecordComponent : ASTNode
    {
        std::vector<Annotation> annotations; // Annotations attached to this syntax element.
        std::u16string name; // Declared or referenced name.
        TypeNode type; // Written component type.
        std::optional<ArrayDimension> varargs; // Present for ...; stores annotations on the ellipsis.
    };

    // record Point<T>(T x, T y) implements I { ... }.
    struct RecordDecl : TypeDecl
    {
        std::vector<GenericParameter> typeParameters; // Declared generic parameters: <T extends A>.
        std::vector<RecordComponent> components; // Components in source order.
        std::vector<TypeNode> interfaces; // Types after implements.
    };
} // javac

#endif //JAVACOMPILER_DECLARATIONS_H
