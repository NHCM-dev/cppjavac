//
// Created by NHCM on 2026/11/22.
//

#include "JavaDef.h"

#include <algorithm>
#include <map>
#include <utility>

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

    std::optional<PrimitiveType> primitiveTypeFromKeyword(const JavaKeyword& keyword)
    {
        static constexpr std::pair<JavaKeyword, PrimitiveType> types[] = {
            {BOOLEAN, PrimitiveType::Boolean},
            {BYTE, PrimitiveType::Byte},
            {SHORT, PrimitiveType::Short},
            {CHAR, PrimitiveType::Char},
            {INT, PrimitiveType::Int},
            {LONG, PrimitiveType::Long},
            {FLOAT, PrimitiveType::Float},
            {DOUBLE, PrimitiveType::Double},
        };
        for (const auto& [candidate, type] : types)
        {
            if (keyword == candidate)
            {
                return type;
            }
        }
        return std::nullopt;
    }

    std::optional<FlagType> flagTypeFromKeyword(const JavaKeyword& keyword)
    {
        static constexpr std::pair<JavaKeyword, FlagType> flags[] = {
            {PUBLIC, FlagType::Public},
            {PROTECTED, FlagType::Protected},
            {PRIVATE, FlagType::Private},
            {ABSTRACT, FlagType::Abstract},
            {STATIC, FlagType::Static},
            {FINAL, FlagType::Final},
            {TRANSIENT, FlagType::Transient},
            {VOLATILE, FlagType::Volatile},
            {SYNCHRONIZED, FlagType::Synchronized},
            {NATIVE, FlagType::Native},
            {STRICTFP, FlagType::Strictfp},
            {DEFAULT, FlagType::Default},
            {SEALED, FlagType::Sealed},
            {NON_SEALED, FlagType::NonSealed},
        };
        for (const auto& [candidate, flag] : flags)
        {
            if (keyword == candidate)
            {
                return flag;
            }
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
            TRUE_LITERAL, FALSE_LITERAL, NULL_LITERAL,
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

    const JavaKeyword* findKeywordUtf16(std::u16string_view word)
    {
        std::string ascii;
        ascii.reserve(word.size());

        for (char16_t ch : word)
        {
            if (ch > 0x7F)
            {
                return nullptr;
            }
            ascii.push_back(static_cast<char>(ch));
        }

        return findKeyword(ascii);
    }

    template <typename Char>
    static bool hasValidAsciiIdentifierCharacters(std::basic_string_view<Char> name, bool allowNumericStart)
    {
        const auto isDigit = [](Char ch) { return ch >= '0' && ch <= '9'; };
        const auto isLetter = [](Char ch) {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_' || ch == '$';
        };

        return !name.empty()
            && (isLetter(name.front()) || (allowNumericStart && isDigit(name.front())))
            && std::ranges::all_of(name.substr(1), [&](Char ch) { return isLetter(ch) || isDigit(ch); });
    }

    bool hasValidIdentifierCharacters(std::string_view name, bool allowNumericStart)
    {
        return hasValidAsciiIdentifierCharacters(name, allowNumericStart);
    }

    enum class IdentifierCharacter
    {
        Invalid,
        Start,
        Part,
        Digit
    };

    struct IdentifierRange
    {
        char32_t first;
        char32_t last;
        IdentifierCharacter kind;
    };

    static IdentifierCharacter getIdentifierCharacter(char32_t codePoint)
    {
        static constexpr IdentifierRange ranges[] = {
#include "JavaIdentifierRanges.inc"
        };
        const auto* range = std::ranges::lower_bound(ranges, codePoint, {}, &IdentifierRange::last);
        if (range != std::end(ranges) && range->first <= codePoint)
        {
            return range->kind;
        }
        return IdentifierCharacter::Invalid;
    }

    std::size_t getJavaIdentifierLength(std::u16string_view source, bool allowNumericStart)
    {
        std::size_t length = 0;
        while (length < source.size())
        {
            char32_t codePoint = source[length];
            std::size_t width = 1;
            if (codePoint >= 0xD800 && codePoint <= 0xDBFF && length + 1 < source.size())
            {
                const char32_t low = source[length + 1];
                if (low >= 0xDC00 && low <= 0xDFFF)
                {
                    codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (low - 0xDC00);
                    width = 2;
                }
            }

            const auto kind = getIdentifierCharacter(codePoint);
            if (kind == IdentifierCharacter::Invalid
                || (length == 0 && kind != IdentifierCharacter::Start
                    && !(allowNumericStart && kind == IdentifierCharacter::Digit)))
            {
                break;
            }
            length += width;
        }
        return length;
    }

    bool hasValidIdentifierCharacters(std::u16string_view name, bool allowNumericStart)
    {
        return !name.empty() && getJavaIdentifierLength(name, allowNumericStart) == name.size();
    }

    template <typename Char>
    static bool isValidUnqualifiedName(std::basic_string_view<Char> name)
    {
        if (name.empty())
        {
            return false;
        }
        for (Char ch : name)
        {
            if (ch == '.' || ch == ';' || ch == '[' || ch == '/')
            {
                return false;
            }
        }
        return true;
    }

    template <typename Char>
    static bool isValidInternalClassName(std::basic_string_view<Char> name)
    {
        while (true)
        {
            const auto separator = name.find(static_cast<Char>('/'));
            if (!isValidUnqualifiedName(name.substr(0, separator)))
            {
                return false;
            }
            if (separator == name.npos)
            {
                return true;
            }
            name.remove_prefix(separator + 1);
        }
    }

    template <typename Char>
    static bool isValidOrdinaryMethodName(std::basic_string_view<Char> name)
    {
        return isValidUnqualifiedName(name)
            && name.find(static_cast<Char>('<')) == name.npos
            && name.find(static_cast<Char>('>')) == name.npos;
    }

    bool isValidJvmInternalClassName(std::string_view name)
    {
        return isValidInternalClassName(name);
    }

    bool isValidJvmInternalClassName(std::u16string_view name)
    {
        return isValidInternalClassName(name);
    }

    bool isValidJvmFieldName(std::string_view name)
    {
        return isValidUnqualifiedName(name);
    }

    bool isValidJvmFieldName(std::u16string_view name)
    {
        return isValidUnqualifiedName(name);
    }

    bool isValidJvmOrdinaryMethodName(std::string_view name)
    {
        return isValidOrdinaryMethodName(name);
    }

    bool isValidJvmOrdinaryMethodName(std::u16string_view name)
    {
        return isValidOrdinaryMethodName(name);
    }
}
