//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_EXTENSION_H
#define JAVACOMPILER_EXTENSION_H

#include <cstdint>
#include <optional>
#include <set>
#include <span>
#include <string_view>

enum class JavacExtension : std::uint8_t
{
    // Naming
    AllowKeywordIdentifiers,       // Allow keywords as identifiers
    AllowNumericStartIdentifiers,  // Allow identifiers starting with digits
    AllowPublicClassNameMismatch,  // Allow public class name != source filename
    AllowMultiplePublicClasses,    // Allow multiple public top-level classes

    // Variables
    AllowVariableShadowing,        // Allow local variables to shadow other locals
    AllowImplicitVariableTypes,    // Allow type inference beyond standard var
    AllowAutoTypeConversion,       // Allow additional implicit type conversions

    // Methods
    AllowOperatorOverloading,      // User-defined operator overloading
    AllowDefaultParameters,        // Default method arguments
    AllowExtensionMethods,         // Extension methods
    AllowLocalFunctions,           // Local function declarations

    // Control Flow
    AllowGoto,                     // Goto-like source-level jumps

    // Other
    AllowUnsignedIntegers,         // Unsigned arithmetic via compiler lowering
    AllowCompileTimeEvaluation,    // Compile-time expression/function evaluation
};

namespace javac
{
    struct ExtensionInfo
    {
        JavacExtension extension;
        std::string_view name;
    };

    [[nodiscard]] std::span<const ExtensionInfo> listExtensions();
    [[nodiscard]] std::optional<JavacExtension> parseExtension(std::string_view name);
    [[nodiscard]] std::set<JavacExtension> parseExtensions(std::string_view value);
}

#endif //JAVACOMPILER_EXTENSION_H
