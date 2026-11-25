//
// Created by NHCM on 2026/11/22.
//

#include "LexerDef.h"

#include <algorithm>

namespace javac
{
    struct SymbolSpelling
    {
        SymbolType type;
        std::string_view spelling;
    };

    static constexpr SymbolSpelling symbolSpellings[] = {
        {SymbolType::LParen, "("},
        {SymbolType::RParen, ")"},
        {SymbolType::LBrace, "{"},
        {SymbolType::RBrace, "}"},
        {SymbolType::LBracket, "["},
        {SymbolType::RBracket, "]"},
        {SymbolType::Question, "?"},
        {SymbolType::Colon, ":"},
        {SymbolType::Semicolon, ";"},
        {SymbolType::Comma, ","},
        {SymbolType::Dot, "."},
        {SymbolType::Ellipsis, "..."},
        {SymbolType::AtSign, "@"},
        {SymbolType::DoubleColon, "::"},
        {SymbolType::Underscore, "_"},
        {SymbolType::Plus, "+"},
        {SymbolType::Minus, "-"},
        {SymbolType::Star, "*"},
        {SymbolType::Slash, "/"},
        {SymbolType::Percent, "%"},
        {SymbolType::PlusPlus, "++"},
        {SymbolType::MinusMinus, "--"},
        {SymbolType::Assign, "="},
        {SymbolType::PlusAssign, "+="},
        {SymbolType::MinusAssign, "-="},
        {SymbolType::StarAssign, "*="},
        {SymbolType::SlashAssign, "/="},
        {SymbolType::PercentAssign, "%="},
        {SymbolType::AmpersandAssign, "&="},
        {SymbolType::PipeAssign, "|="},
        {SymbolType::CaretAssign, "^="},
        {SymbolType::LeftShiftAssign, "<<="},
        {SymbolType::RightShiftAssign, ">>="},
        {SymbolType::UnsignedRightShiftAssign, ">>>="},
        {SymbolType::EqualEqual, "=="},
        {SymbolType::BangEqual, "!="},
        {SymbolType::LAngleBracket, "<"},
        {SymbolType::LessEqual, "<="},
        {SymbolType::RAngleBracket, ">"},
        {SymbolType::GreaterEqual, ">="},
        {SymbolType::Bang, "!"},
        {SymbolType::AmpersandAmpersand, "&&"},
        {SymbolType::PipePipe, "||"},
        {SymbolType::Ampersand, "&"},
        {SymbolType::Pipe, "|"},
        {SymbolType::Caret, "^"},
        {SymbolType::Tilde, "~"},
        {SymbolType::LeftShift, "<<"},
        {SymbolType::RightShift, ">>"},
        {SymbolType::UnsignedRightShift, ">>>"},
        {SymbolType::Arrow, "->"},
    };

    template <typename Char>
    static std::optional<SymbolType> lookupSymbol(std::basic_string_view<Char> spelling)
    {
        const auto* match = std::ranges::find_if(symbolSpellings, [spelling](const SymbolSpelling& entry) {
            return std::ranges::equal(entry.spelling, spelling);
        });

        if (match == std::end(symbolSpellings))
        {
            return std::nullopt;
        }
        return match->type;
    }

    std::optional<SymbolType> findSymbol(std::string_view spelling)
    {
        return lookupSymbol(spelling);
    }

    std::optional<SymbolType> findSymbol(std::u16string_view spelling)
    {
        return lookupSymbol(spelling);
    }

    bool isSymbol(std::string_view spelling, SymbolType expected)
    {
        return findSymbol(spelling) == expected;
    }

    bool isSymbol(std::u16string_view spelling, SymbolType expected)
    {
        return findSymbol(spelling) == expected;
    }

    template <typename Char>
    static bool isStandaloneSymbolSpelling(std::basic_string_view<Char> spelling)
    {
        if (!lookupSymbol(spelling))
        {
            return false;
        }

        for (const auto& entry : symbolSpellings)
        {
            const auto other = entry.spelling;
            if (other.size() > spelling.size() && !std::ranges::search(other, spelling).empty())
            {
                return false;
            }

            const auto maxOverlap = std::min(spelling.size(), other.size() - 1);
            for (std::size_t length = 1; length <= maxOverlap; ++length)
            {
                if (std::ranges::equal(spelling.substr(0, length), other.substr(other.size() - length))
                    || std::ranges::equal(spelling.substr(spelling.size() - length), other.substr(0, length)))
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool isStandaloneSymbol(std::string_view spelling)
    {
        return isStandaloneSymbolSpelling(spelling);
    }

    bool isStandaloneSymbol(std::u16string_view spelling)
    {
        return isStandaloneSymbolSpelling(spelling);
    }

    std::string_view invalidTokenStr(InvalidTokenType type)
    {
        switch (type)
        {
            case InvalidTokenType::SwitchLineInStringLiteral:
                return "line break in string literal";
            case InvalidTokenType::SwitchLineInCharacterLiteral:
                return "line break in character literal";
            case InvalidTokenType::NoneFinishEscapeInStringLiteral:
                return "incomplete escape sequence in string literal";
            case InvalidTokenType::NoneFinishEscapeInCharacterLiteral:
                return "incomplete escape sequence in character literal";
            case InvalidTokenType::NoneFinishStringLiteral:
                return "unterminated string literal";
            case InvalidTokenType::NoneFinishCharacterLiteral:
                return "unterminated character literal";
            case InvalidTokenType::NoneFinishMultiLineComment:
                return "unterminated block comment";
            case InvalidTokenType::TooLargeCharacterLiteral:
                return "too many characters in character literal";
            case InvalidTokenType::UnknownTokenType:
                return "unrecognized token";
            case InvalidTokenType::EmptyCharacterLiteral:
                return "empty character literal";
            case InvalidTokenType::InvalidEscapeSequence:
                return "invalid escape sequence";
            case InvalidTokenType::InvalidNumericLiteral:
                return "invalid numeric literal";
        }
        return "invalid token";
    }
} // javac
