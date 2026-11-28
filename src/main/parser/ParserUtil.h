//
// Created by NHCM on 2026/11/25.
//

#ifndef CPPJAVAC_PARSERUTIL_H
#define CPPJAVAC_PARSERUTIL_H
#include <string_view>

#include "ParserException.h"
#include "ast/ASTDef.h"
#include "main/core/def/JavaDef.h"

namespace jcutils
{
    /**
     * Parse a Java integer literal and return its signed value.
     * @throw std::invalid_argument if entered string is invalid
     * @throw std::out_of_range if the value is outside the literal's int or long range
     * @param str UTF-16 integer with an optional sign and l/L suffix
     * @return parsed int64_t result
     */
    javac::jlong int64FromString(std::u16string_view str);
    // Check the l/L suffix when a spelling is provided, otherwise check the int32_t range.
    bool int64IsLong(javac::jlong val, std::u16string_view spelling = {});

    /**
     * Parse a decimal or hexadecimal floating-point number as a double.
     * @throw std::invalid_argument if str has invalid syntax
     * @throw std::out_of_range if the value overflows or underflows to zero
     * @param str UTF-16 number with an optional sign and Java floating-point suffix
     * @return parsed double value
     */
    javac::jdouble doubleFromString(std::u16string_view str);

    // Parse at float precision with the same syntax and exceptions as doubleFromString.
    javac::jfloat floatFromString(std::u16string_view str);

    // Return true if the spelling is a Java floating-point literal with an f/F suffix.
    bool is32Float(std::u16string_view str);

    javac::jboolean boolFromString(std::u16string_view input);

    bool isActualClass(const javac::TypeNode& typeNode);
    bool isPrimitive(const javac::TypeNode& typeNode);

    // Check the written type form for extends, implements or a declared type parameter bound.
    bool canExtImplTo(const javac::TypeNode& typeNode);
    // Check the type name in a permits clause.
    bool canPermit(const javac::TypeNode& typeNode);
    // Check a type argument or wildcard bound. Allow primitives only for exact arguments.
    bool canGeneric(const javac::TypeNode& typeNode);


    bool haveModifier(const std::vector<javac::ModifierFlag>& modifiers, javac::FlagType flag);

    enum class ClassDeclContext
    {
        TopLevel,
        Member,
        Anonymous,
        WithinInterface, // Member class declared in an interface.
        WithinAnnotation, // Member class declared in an annotation interface.
        Local
    };

    [[nodiscard]] constexpr bool isInterfaceBody(ClassDeclContext context)
    {
        return context == ClassDeclContext::WithinInterface || context == ClassDeclContext::WithinAnnotation;
    }

    enum class MethodDeclContext
    {
        Normal,
        WithinAbstractClass,
        WithinInterface, // method i
        WithinAnnotation
    };

    const javac::ModifierFlag* findInvalidClassModifier(const std::vector<javac::ModifierFlag>& modifiers,
        javac::LangLevel langLevel = javac::LangLevel::Eight,
        ClassDeclContext context = ClassDeclContext::TopLevel);
    const javac::ModifierFlag* findInvalidFieldModifier(const std::vector<javac::ModifierFlag>& modifiers,
        ClassDeclContext context = ClassDeclContext::TopLevel);
    const javac::ModifierFlag* findInvalidMethodModifier(const std::vector<javac::ModifierFlag>& modifiers,
        MethodDeclContext context = MethodDeclContext::Normal,
        javac::LangLevel langLevel = javac::LangLevel::Eight);
    const javac::ModifierFlag* findInvalidConstructorModifier(const std::vector<javac::ModifierFlag>& modifiers);

    enum class ModifierDeclaration
    {
        Class,
        Field,
        Method,
        Constructor
    };

    struct ModifierConflict
    {
        javac::ParserExceptionType exception;
        std::optional<SourceRange> range;
    };

    // Duplicate errors take priority over access conflicts and declaration-specific conflicts.
    [[nodiscard]] std::optional<ModifierConflict> findModifierConflict(
        const std::vector<javac::ModifierFlag>& modifiers, ModifierDeclaration declaration);

    enum class StatementContext
    {
        Embedded,
        Block
    };

    enum class VariableDeclContext
    {
        Local,
        EnhancedFor,
        TryResource
    };

} // jcutils

#endif //CPPJAVAC_PARSERUTIL_H
