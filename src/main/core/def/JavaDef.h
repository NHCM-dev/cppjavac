//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_JAVADEF_H
#define JAVACOMPILER_JAVADEF_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
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
        Reserved,
        ReservedUnused, // const and goto
        Contextual,     // Recognized as a keyword in specific syntactic contexts
        BooleanLiteral,
        NullLiteral
    };

    class JavaKeyword
    {
        const std::string_view string;
        const LangLevel minLangLevel;
        const JavaKeywordKind kind;

    public:
        constexpr JavaKeyword(std::string_view string,
            LangLevel minLangLevel = LangLevel::Eight, JavaKeywordKind kind = JavaKeywordKind::Reserved)
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
    inline constexpr JavaKeyword CONST{"const", LangLevel::Eight, JavaKeywordKind::ReservedUnused};
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
    inline constexpr JavaKeyword GOTO{"goto", LangLevel::Eight, JavaKeywordKind::ReservedUnused};
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

    // Fixed literal spellings share the lookup table with keywords.
    inline constexpr JavaKeyword TRUE_LITERAL{"true", LangLevel::Eight, JavaKeywordKind::BooleanLiteral};
    inline constexpr JavaKeyword FALSE_LITERAL{"false", LangLevel::Eight, JavaKeywordKind::BooleanLiteral};
    inline constexpr JavaKeyword NULL_LITERAL{"null", LangLevel::Eight, JavaKeywordKind::NullLiteral};

    // Java 11 (features introduced in Java 9–11)
    inline constexpr JavaKeyword UNDERSCORE{"_", LangLevel::Eleven};

    // Module system
    inline constexpr JavaKeyword MODULE{"module", LangLevel::Eleven, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword OPEN{"open", LangLevel::Eleven, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword OPENS{"opens", LangLevel::Eleven, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword REQUIRES{"requires", LangLevel::Eleven, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword TRANSITIVE{"transitive", LangLevel::Eleven, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword EXPORTS{"exports", LangLevel::Eleven, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword TO{"to", LangLevel::Eleven, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword USES{"uses", LangLevel::Eleven, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword PROVIDES{"provides", LangLevel::Eleven, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword WITH{"with", LangLevel::Eleven, JavaKeywordKind::Contextual};

    // Local variable type inference
    inline constexpr JavaKeyword VAR{"var", LangLevel::Eleven, JavaKeywordKind::Contextual};

    // Java 17 (features introduced in Java 12–17)
    inline constexpr JavaKeyword YIELD{"yield", LangLevel::Seventeen, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword RECORD{"record", LangLevel::Seventeen, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword SEALED{"sealed", LangLevel::Seventeen, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword PERMITS{"permits", LangLevel::Seventeen, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword NON_SEALED{"non-sealed", LangLevel::Seventeen, JavaKeywordKind::Contextual};

    // Java 21
    inline constexpr JavaKeyword WHEN{"when", LangLevel::TwentyOne, JavaKeywordKind::Contextual};

    // Looks up keywords and fixed literal spellings. The caller checks
    // the language level and decides whether a contextual word is a keyword in its current context.
    [[nodiscard]]
    const JavaKeyword* findKeyword(std::string_view spelling);

    [[nodiscard]]
    const JavaKeyword* findKeywordUtf16(std::u16string_view word);

    // The narrow overload checks ASCII. The UTF-16 overload uses Java 8 Unicode rules.
    [[nodiscard]] bool hasValidIdentifierCharacters(std::string_view name, bool allowNumericStart = false);
    [[nodiscard]] bool hasValidIdentifierCharacters(std::u16string_view name, bool allowNumericStart = false);
    [[nodiscard]] std::size_t getJavaIdentifierLength(std::u16string_view source, bool allowNumericStart = false);

    // JVM class-file name syntax. The input is assumed to be decoded text.
    // Class names use '/' between components, e.g. java/lang/String.
    [[nodiscard]] bool isValidJvmInternalClassName(std::string_view name);
    [[nodiscard]] bool isValidJvmInternalClassName(std::u16string_view name);
    [[nodiscard]] bool isValidJvmFieldName(std::string_view name);
    [[nodiscard]] bool isValidJvmFieldName(std::u16string_view name);
    // Ordinary method names exclude the special names <init> and <clinit>.
    [[nodiscard]] bool isValidJvmOrdinaryMethodName(std::string_view name);
    [[nodiscard]] bool isValidJvmOrdinaryMethodName(std::u16string_view name);
}

#endif //JAVACOMPILER_JAVADEF_H
