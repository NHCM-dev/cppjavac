//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_JAVADEF_H
#define JAVACOMPILER_JAVADEF_H

#include <cstdint>
#include <optional>
#include <string_view>

namespace javac
{
    enum class DebugInfo
    {
        Lines, // Line number
        Vars, // Local variable
        Source // Source file
    };

    enum class LangLevel : std::uint8_t
    {
        Eight = 8,
        Eleven = 11,
        Seventeen = 17,
        TwentyOne = 21,
        TwentyFive = 25
    };

    [[nodiscard]] std::optional<LangLevel> parseLangLevel(int level);

    enum class JavaKeywordKind : std::uint8_t
    {
        Normal,      // Ordinary reserved keyword
        Prohibited,  // Reserved but unused
        Restricted   // Context-sensitive keyword
    };

    class JavaKeyword
    {
        const std::string_view string;
        const LangLevel minLangLevel;
        const JavaKeywordKind kind;

    public:
        constexpr JavaKeyword(std::string_view string,
            LangLevel minLangLevel = LangLevel::Eight, JavaKeywordKind kind = JavaKeywordKind::Normal)
            : string(string),
              minLangLevel(minLangLevel),
              kind(kind) {}

        constexpr bool operator==(std::string_view str) const
        {
            return string == str;
        }

        [[nodiscard]]
        constexpr std::string_view getString() const
        {
            return string;
        }

        [[nodiscard]]
        constexpr LangLevel getMinLangLevel() const
        {
            return minLangLevel;
        }

        [[nodiscard]]
        constexpr JavaKeywordKind getKind() const
        {
            return kind;
        }

        [[nodiscard]]
        constexpr bool isSupported(LangLevel level) const
        {
            return level >= minLangLevel;
        }
    };


    inline constexpr JavaKeyword ABSTRACT{"abstract"};
    inline constexpr JavaKeyword ASSERT{"assert"};
    inline constexpr JavaKeyword BOOLEAN{"boolean"};
    inline constexpr JavaKeyword BREAK{"break"};
    inline constexpr JavaKeyword BYTE{"byte"};
    inline constexpr JavaKeyword CASE{"case"};
    inline constexpr JavaKeyword CATCH{"catch"};
    inline constexpr JavaKeyword CHAR{"char"};
    inline constexpr JavaKeyword CLASS{"class"};
    inline constexpr JavaKeyword CONST{"const", LangLevel::Eight, JavaKeywordKind::Prohibited};
    inline constexpr JavaKeyword CONTINUE{"continue"};
    inline constexpr JavaKeyword DEFAULT{"default"};
    inline constexpr JavaKeyword DO{"do"};
    inline constexpr JavaKeyword DOUBLE{"double"};
    inline constexpr JavaKeyword ELSE{"else"};
    inline constexpr JavaKeyword ENUM{"enum"};
    inline constexpr JavaKeyword EXTENDS{"extends"};
    inline constexpr JavaKeyword FINAL{"final"};
    inline constexpr JavaKeyword FINALLY{"finally"};
    inline constexpr JavaKeyword FLOAT{"float"};
    inline constexpr JavaKeyword FOR{"for"};
    inline constexpr JavaKeyword GOTO{"goto", LangLevel::Eight, JavaKeywordKind::Prohibited};
    inline constexpr JavaKeyword IF{"if"};
    inline constexpr JavaKeyword IMPLEMENTS{"implements"};
    inline constexpr JavaKeyword IMPORT{"import"};
    inline constexpr JavaKeyword INSTANCEOF{"instanceof"};
    inline constexpr JavaKeyword INT{"int"};
    inline constexpr JavaKeyword INTERFACE{"interface"};
    inline constexpr JavaKeyword LONG{"long"};
    inline constexpr JavaKeyword NATIVE{"native"};
    inline constexpr JavaKeyword NEW{"new"};
    inline constexpr JavaKeyword PACKAGE{"package"};
    inline constexpr JavaKeyword PRIVATE{"private"};
    inline constexpr JavaKeyword PROTECTED{"protected"};
    inline constexpr JavaKeyword PUBLIC{"public"};
    inline constexpr JavaKeyword RETURN{"return"};
    inline constexpr JavaKeyword SHORT{"short"};
    inline constexpr JavaKeyword STATIC{"static"};
    inline constexpr JavaKeyword STRICTFP{"strictfp"};
    inline constexpr JavaKeyword SUPER{"super"};
    inline constexpr JavaKeyword SWITCH{"switch"};
    inline constexpr JavaKeyword SYNCHRONIZED{"synchronized"};
    inline constexpr JavaKeyword THIS{"this"};
    inline constexpr JavaKeyword THROW{"throw"};
    inline constexpr JavaKeyword THROWS{"throws"};
    inline constexpr JavaKeyword TRANSIENT{"transient"};
    inline constexpr JavaKeyword TRY{"try"};
    inline constexpr JavaKeyword VOID{"void"};
    inline constexpr JavaKeyword VOLATILE{"volatile"};
    inline constexpr JavaKeyword WHILE{"while"};

    // Java 11 (features introduced in Java 9–11)
    inline constexpr JavaKeyword UNDERSCORE{"_", LangLevel::Eleven};

    // Module system
    inline constexpr JavaKeyword MODULE{"module", LangLevel::Eleven, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword OPEN{"open", LangLevel::Eleven, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword OPENS{"opens", LangLevel::Eleven, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword REQUIRES{"requires", LangLevel::Eleven, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword TRANSITIVE{"transitive", LangLevel::Eleven, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword EXPORTS{"exports", LangLevel::Eleven, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword TO{"to", LangLevel::Eleven, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword USES{"uses", LangLevel::Eleven, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword PROVIDES{"provides", LangLevel::Eleven, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword WITH{"with", LangLevel::Eleven, JavaKeywordKind::Restricted};

    // Local variable type inference
    inline constexpr JavaKeyword VAR{"var", LangLevel::Eleven, JavaKeywordKind::Restricted};

    // Java 17 (features introduced in Java 12–17)
    inline constexpr JavaKeyword YIELD{"yield", LangLevel::Seventeen, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword RECORD{"record", LangLevel::Seventeen, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword SEALED{"sealed", LangLevel::Seventeen, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword PERMITS{"permits", LangLevel::Seventeen, JavaKeywordKind::Restricted};
    inline constexpr JavaKeyword NON_SEALED{"non-sealed", LangLevel::Seventeen, JavaKeywordKind::Restricted};

    // Java 21
    inline constexpr JavaKeyword WHEN{"when", LangLevel::TwentyOne, JavaKeywordKind::Restricted};

    // Looks up the spelling, including restricted keywords. The caller checks
    // the language level and decides whether a restricted word is a keyword in context.
    [[nodiscard]] const JavaKeyword* findKeyword(std::string_view spelling);
}

#endif //JAVACOMPILER_JAVADEF_H
