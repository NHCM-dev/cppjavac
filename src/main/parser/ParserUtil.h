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
    bool canGeneric(const javac::TypeNode& typeNode, bool allowPrimitive = false);


    bool haveModifier(const std::vector<javac::ModifierFlag>& modifiers, javac::FlagType flag);

    // === methods to detect whenever a vector of modifiers contains invalid elements for some types ===
    // The determination is context less

    bool modifiersAllowedForClass(std::vector<javac::ModifierFlag>& modifiers, javac::LangLevel langLevel = javac::LangLevel::Eight);
    bool modifiersAllowedForField(std::vector<javac::ModifierFlag>& modifiers);
    bool modifiersAllowedForMethod(std::vector<javac::ModifierFlag>& modifiers);
    bool modifiersAllowedForConstructor(std::vector<javac::ModifierFlag>& modifiers);

    enum class ModifierDeclaration
    {
        Class,
        Field,
        Method,
        Constructor
    };

    // Returns the first duplicate, or a conflict in the declaration's keyword modifiers.
    [[nodiscard]] std::optional<javac::ParserExceptionType> findModifierConflict(
        const std::vector<javac::ModifierFlag>& modifiers, ModifierDeclaration declaration);


} // jcutils

#endif //CPPJAVAC_PARSERUTIL_H
