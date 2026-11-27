//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_JAVADEF_H
#define JAVACOMPILER_JAVADEF_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace javac
{
    using jbyte    = std::int8_t;
    using jshort   = std::int16_t;
    using jchar    = char16_t;
    using jint     = std::int32_t;
    using jlong    = std::int64_t;
    using jfloat   = float;
    using jdouble  = double;
    using jboolean = bool;

    static_assert(sizeof(jfloat) == 4);
    static_assert(sizeof(jdouble) == 8);

    inline constexpr int32_t JINT_MIN = std::numeric_limits<int32_t>::min();
    inline constexpr int32_t JINT_MAX = std::numeric_limits<int32_t>::max();

    inline constexpr int64_t JLONG_MIN = std::numeric_limits<int64_t>::min();
    inline constexpr int64_t JLONG_MAX = std::numeric_limits<int64_t>::max();

    enum class DebugInfo
    {
        Lines, // Line number
        Vars, // Local variable
        Source // Source file
    };

    enum class LangLevel : std::uint8_t
    {
        One = 1,  // Java 1.0
        Two = 2,  // Java 1.2
        Four = 4, // Java 1.4
        Five = 5,
        Eight = 8,
        Nine = 9,
        Ten = 10,
        Eleven = 11,
        Fourteen = 14,
        Sixteen = 16,
        Seventeen = 17,
        TwentyOne = 21,
        TwentyFive = 25
    };

    [[nodiscard]]
    std::optional<LangLevel> parseLangLevel(int level);

    enum class PrimitiveType : std::uint8_t
    {
        Boolean,
        Byte,
        Short,
        Char,
        Int,
        Long,
        Float,
        Double,
    };

    enum class FlagType : std::uint8_t
    {
        Public,
        Protected,
        Private,
        Abstract,
        Static,
        Final,
        Transient,
        Volatile,
        Synchronized,
        Native,
        Strictfp,
        Default,
        Sealed,
        NonSealed,
    };

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

        constexpr bool operator==(const JavaKeyword& keyword) const
        {
            return string == keyword.string;
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


    inline constexpr JavaKeyword ABSTRACT{"abstract", LangLevel::One};
    inline constexpr JavaKeyword ASSERT{"assert", LangLevel::Four};
    inline constexpr JavaKeyword BOOLEAN{"boolean", LangLevel::One};
    inline constexpr JavaKeyword BREAK{"break", LangLevel::One};
    inline constexpr JavaKeyword BYTE{"byte", LangLevel::One};
    inline constexpr JavaKeyword CASE{"case", LangLevel::One};
    inline constexpr JavaKeyword CATCH{"catch", LangLevel::One};
    inline constexpr JavaKeyword CHAR{"char", LangLevel::One};
    inline constexpr JavaKeyword CLASS{"class", LangLevel::One};
    inline constexpr JavaKeyword CONST{"const", LangLevel::One, JavaKeywordKind::ReservedUnused};
    inline constexpr JavaKeyword CONTINUE{"continue", LangLevel::One};
    inline constexpr JavaKeyword DEFAULT{"default", LangLevel::One};
    inline constexpr JavaKeyword DO{"do", LangLevel::One};
    inline constexpr JavaKeyword DOUBLE{"double", LangLevel::One};
    inline constexpr JavaKeyword ELSE{"else", LangLevel::One};
    inline constexpr JavaKeyword ENUM{"enum", LangLevel::Five};
    inline constexpr JavaKeyword EXTENDS{"extends", LangLevel::One};
    inline constexpr JavaKeyword FINAL{"final", LangLevel::One};
    inline constexpr JavaKeyword FINALLY{"finally", LangLevel::One};
    inline constexpr JavaKeyword FLOAT{"float", LangLevel::One};
    inline constexpr JavaKeyword FOR{"for", LangLevel::One};
    inline constexpr JavaKeyword GOTO{"goto", LangLevel::One, JavaKeywordKind::ReservedUnused};
    inline constexpr JavaKeyword IF{"if", LangLevel::One};
    inline constexpr JavaKeyword IMPLEMENTS{"implements", LangLevel::One};
    inline constexpr JavaKeyword IMPORT{"import", LangLevel::One};
    inline constexpr JavaKeyword INSTANCEOF{"instanceof", LangLevel::One};
    inline constexpr JavaKeyword INT{"int", LangLevel::One};
    inline constexpr JavaKeyword INTERFACE{"interface", LangLevel::One};
    inline constexpr JavaKeyword LONG{"long", LangLevel::One};
    inline constexpr JavaKeyword NATIVE{"native", LangLevel::One};
    inline constexpr JavaKeyword NEW{"new", LangLevel::One};
    inline constexpr JavaKeyword PACKAGE{"package", LangLevel::One};
    inline constexpr JavaKeyword PRIVATE{"private", LangLevel::One};
    inline constexpr JavaKeyword PROTECTED{"protected", LangLevel::One};
    inline constexpr JavaKeyword PUBLIC{"public", LangLevel::One};
    inline constexpr JavaKeyword RETURN{"return", LangLevel::One};
    inline constexpr JavaKeyword SHORT{"short", LangLevel::One};
    inline constexpr JavaKeyword STATIC{"static", LangLevel::One};
    inline constexpr JavaKeyword STRICTFP{"strictfp", LangLevel::Two};
    inline constexpr JavaKeyword SUPER{"super", LangLevel::One};
    inline constexpr JavaKeyword SWITCH{"switch", LangLevel::One};
    inline constexpr JavaKeyword SYNCHRONIZED{"synchronized", LangLevel::One};
    inline constexpr JavaKeyword THIS{"this", LangLevel::One};
    inline constexpr JavaKeyword THROW{"throw", LangLevel::One};
    inline constexpr JavaKeyword THROWS{"throws", LangLevel::One};
    inline constexpr JavaKeyword TRANSIENT{"transient", LangLevel::One};
    inline constexpr JavaKeyword TRY{"try", LangLevel::One};
    inline constexpr JavaKeyword VOID{"void", LangLevel::One};
    inline constexpr JavaKeyword VOLATILE{"volatile", LangLevel::One};
    inline constexpr JavaKeyword WHILE{"while", LangLevel::One};

    // Fixed literal spellings share the lookup table with keywords.
    inline constexpr JavaKeyword TRUE_LITERAL{"true", LangLevel::One, JavaKeywordKind::BooleanLiteral};
    inline constexpr JavaKeyword FALSE_LITERAL{"false", LangLevel::One, JavaKeywordKind::BooleanLiteral};
    inline constexpr JavaKeyword NULL_LITERAL{"null", LangLevel::One, JavaKeywordKind::NullLiteral};

    // Java 9
    inline constexpr JavaKeyword UNDERSCORE{"_", LangLevel::Nine};

    // Module system
    inline constexpr JavaKeyword MODULE{"module", LangLevel::Nine, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword OPEN{"open", LangLevel::Nine, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword OPENS{"opens", LangLevel::Nine, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword REQUIRES{"requires", LangLevel::Nine, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword TRANSITIVE{"transitive", LangLevel::Nine, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword EXPORTS{"exports", LangLevel::Nine, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword TO{"to", LangLevel::Nine, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword USES{"uses", LangLevel::Nine, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword PROVIDES{"provides", LangLevel::Nine, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword WITH{"with", LangLevel::Nine, JavaKeywordKind::Contextual};

    // Local variable type inference
    inline constexpr JavaKeyword VAR{"var", LangLevel::Ten, JavaKeywordKind::Contextual};

    // Switch expressions and records
    inline constexpr JavaKeyword YIELD{"yield", LangLevel::Fourteen, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword RECORD{"record", LangLevel::Sixteen, JavaKeywordKind::Contextual};

    // Sealed types
    inline constexpr JavaKeyword SEALED{"sealed", LangLevel::Seventeen, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword PERMITS{"permits", LangLevel::Seventeen, JavaKeywordKind::Contextual};
    inline constexpr JavaKeyword NON_SEALED{"non-sealed", LangLevel::Seventeen, JavaKeywordKind::Contextual};

    // Java 21
    inline constexpr JavaKeyword WHEN{"when", LangLevel::TwentyOne, JavaKeywordKind::Contextual};

    [[nodiscard]] std::optional<PrimitiveType> primitiveTypeFromKeyword(const JavaKeyword& keyword);
    [[nodiscard]] std::optional<FlagType> flagTypeFromKeyword(const JavaKeyword& keyword);

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
