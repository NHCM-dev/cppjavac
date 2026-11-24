//
// Created by NHCM on 2026/11/22.
//

#include "Extension.h"

#include <stdexcept>
#include <string>

namespace javac
{
    std::span<const ExtensionInfo> listExtensions()
    {
        using enum JavacExtension;
        static constexpr ExtensionInfo extensions[] = {
            {AllowKeywordIdentifiers, "keyword-identifiers"},
            {AllowNumericStartIdentifiers, "numeric-start-identifiers"},
            {AllowPublicClassNameMismatch, "public-class-name-mismatch"},
            {AllowMultiplePublicClasses, "multiple-public-classes"},
            {AllowVariableShadowing, "variable-shadowing"},
            {AllowImplicitVariableTypes, "implicit-variable-types"},
            {AllowAutoTypeConversion, "auto-type-conversion"},
            {AllowOperatorOverloading, "operator-overloading"},
            {AllowDefaultParameters, "default-parameters"},
            {AllowExtensionMethods, "extension-methods"},
            {AllowLocalFunctions, "local-functions"},
            {AllowGoto, "goto"},
            {AllowPrimitiveGenerics, "primitive-generics"},
            {AllowUnsignedIntegers, "unsigned-integers"},
            {AllowCompileTimeEvaluation, "compile-time-evaluation"}
        };
        return extensions;
    }

    std::optional<JavacExtension> parseExtension(std::string_view name)
    {
        for (const auto& [extension, extensionName] : listExtensions())
        {
            if (name == extensionName)
            {
                return extension;
            }
        }
        return std::nullopt;
    }

    std::set<JavacExtension> parseExtensions(std::string_view value)
    {
        std::set<JavacExtension> result;
        if (value == "all")
        {
            for (const auto& info : listExtensions())
            {
                result.insert(info.extension);
            }
            return result;
        }

        while (true)
        {
            const auto comma = value.find(',');
            const auto name = value.substr(0, comma);
            if (name.empty())
            {
                throw std::invalid_argument("Empty extension name");
            }
            const auto extension = parseExtension(name);
            if (!extension)
            {
                throw std::invalid_argument("Unknown extension \"" + std::string(name)
                    + "\". Use --list-extensions to see valid names");
            }
            result.insert(*extension);
            if (comma == std::string_view::npos)
            {
                return result;
            }
            value.remove_prefix(comma + 1);
        }
    }
}
