//
// Created by NHCM on 2026/11/22.
//

#include "JavaDef.h"

#include <map>

namespace javac
{
    std::optional<LangLevel> parseLangLevel(int level)
    {
        // Only Java 8 is currently supported by the compiler.
        if (level == static_cast<int>(LangLevel::Eight))
        {
            return LangLevel::Eight;
        }
        return std::nullopt;
    }

    static const std::map<std::string_view, JavaKeyword> keywords = [] {
        constexpr JavaKeyword entries[] = {
            ABSTRACT, ASSERT, BOOLEAN, BREAK, BYTE, CASE, CATCH, CHAR, CLASS, CONST,
            CONTINUE, DEFAULT, DO, DOUBLE, ELSE, ENUM, EXTENDS, FINAL, FINALLY, FLOAT,
            FOR, GOTO, IF, IMPLEMENTS, IMPORT, INSTANCEOF, INT, INTERFACE, LONG, NATIVE,
            NEW, PACKAGE, PRIVATE, PROTECTED, PUBLIC, RETURN, SHORT, STATIC, STRICTFP,
            SUPER, SWITCH, SYNCHRONIZED, THIS, THROW, THROWS, TRANSIENT, TRY, VOID,
            VOLATILE, WHILE,
            UNDERSCORE, MODULE, OPEN, OPENS, REQUIRES, TRANSITIVE, EXPORTS, TO, USES,
            PROVIDES, WITH, VAR,
            YIELD, RECORD, SEALED, PERMITS, NON_SEALED,
            WHEN
        };
        std::map<std::string_view, JavaKeyword> index;
        for (const auto& keyword : entries)
        {
            index.emplace(keyword.getString(), keyword);
        }
        return index;
    }();

    const JavaKeyword* findKeyword(std::string_view spelling)
    {
        const auto it = keywords.find(spelling);
        return it == keywords.end() ? nullptr : &it->second;
    }
}
