//
// Created by NHCM on 2026/11/22.
//

#include "Lexer.h"

#include <stdexcept>
#include <utility>

#include "main/util/MathUtil.h"

namespace javac
{
    static std::u16string translateUnicodeEscapes(std::u16string_view input,
        std::vector<std::size_t>& sourceOffsets)
    {
        constexpr std::u16string_view hexDigits = u"0123456789abcdefABCDEF";
        std::u16string translated;
        translated.reserve(input.size());
        sourceOffsets.reserve(input.size());
        bool oddBackslashes = false;
        for (std::size_t offset = 0; offset < input.size();)
        {
            const std::size_t start = offset;
            char16_t ch = input[offset++];
            if (ch == u'\\' && !oddBackslashes && offset < input.size() && input[offset] == u'u')
            {
                while (offset < input.size() && input[offset] == u'u')
                {
                    ++offset;
                }
                ch = 0;
                for (int digit = 0; digit < 4; ++digit)
                {
                    const auto value = offset < input.size() ? hexDigits.find(input[offset]) : hexDigits.npos;
                    if (value == hexDigits.npos)
                    {
                        throw std::invalid_argument("Invalid Unicode escape at UTF-16 offset " + std::to_string(start));
                    }
                    ch = static_cast<char16_t>((ch << 4) | (value < 16 ? value : value - 6));
                    ++offset;
                }
                // The escape ends in a raw hex digit, even when its value is a backslash.
                oddBackslashes = false;
            }
            else
            {
                oddBackslashes = ch == u'\\' && !oddBackslashes;
            }
            translated.push_back(ch);
            sourceOffsets.push_back(start);
        }
        return translated;
    }

    Lexer::Lexer(std::u16string_view sourceCode, std::set<JavacExtension> extensions,
        LangLevel sourceLangLevel)
        : sourceCode(sourceCode), extensions(std::move(extensions)), sourceLangLevel(sourceLangLevel)
    {
        keywordIdentifiers = this->extensions.contains(JavacExtension::AllowKeywordIdentifiers);
        numericStart = this->extensions.contains(JavacExtension::AllowNumericStartIdentifiers);
    }

    void Lexer::pushToken(TokenType type, std::optional<SymbolType> symbol,
        std::optional<JavaKeyword> keyword, std::optional<InvalidTokenType> invalidReason,
        bool canBeIdentifier)
    {
        tokens.emplace_back(type, tokenBuffer, tokenStart, symbol, keyword, invalidReason, canBeIdentifier);
        tokenBuffer.clear();
    }

    TokenStream Lexer::lex(bool spaceAndLine, bool comment)
    {
        tokens.clear();
        invalidTokens.clear();
        tokenBuffer.clear();
        offset = 0;
        line = 1;
        column = 0;
        needEndString = false;
        needEndChar = false;
        needEndComment = false;
        isEscaping = false;
        needEndCommentType = TokenType::Invalid;

        std::size_t literalLength = 0;
        bool invalidEscape = false;
        std::vector<std::size_t> sourceOffsets;
        std::u16string translated;
        std::u16string_view source = sourceCode;
        if (source.find(u"\\u") != std::u16string_view::npos)
        {
            translated = translateUnicodeEscapes(source, sourceOffsets);
            source = translated;
        }
        if (source.ends_with(u'\x001A'))
        {
            source.remove_suffix(1);
        }
        const std::size_t sourceSize = source.size();

        const auto isDecimalDigit = [](char16_t ch) { return ch >= u'0' && ch <= u'9'; };
        const auto isHexDigit = [&](char16_t ch) {
            return isDecimalDigit(ch) || (ch >= u'a' && ch <= u'f') || (ch >= u'A' && ch <= u'F');
        };
        const auto isWhitespace = [](char16_t ch) { return ch == u' ' || ch == u'\t' || ch == u'\f'; };
        const auto pushInvalid = [&](InvalidTokenType reason) {
            pushToken(TokenType::Invalid, std::nullopt, std::nullopt, reason);
            invalidTokens.push_back(std::make_unique<Token>(tokens.back()));
        };
        const auto finishComment = [&] {
            if (comment)
            {
                pushToken(needEndCommentType);
            }
            else
            {
                tokenBuffer.clear();
            }
            needEndComment = false;
            needEndCommentType = TokenType::Invalid;
        };

        for (; offset < sourceSize; ++offset)
        {
            const char16_t currentChar = source[offset];
            const char16_t nextChar = offset + 1 < sourceSize ? source[offset + 1] : u'\0';
            const bool newLine = currentChar == u'\n' || currentChar == u'\r';

            if (newLine && needEndComment && needEndCommentType == TokenType::LineComment)
            {
                finishComment();
            }

            if (newLine && (needEndString || needEndChar))
            {
                if (isEscaping)
                {
                    pushInvalid(needEndChar ? InvalidTokenType::NoneFinishEscapeInCharacterLiteral
                                            : InvalidTokenType::NoneFinishEscapeInStringLiteral);
                }
                else
                {
                    pushInvalid(needEndChar ? InvalidTokenType::SwitchLineInCharacterLiteral
                                            : InvalidTokenType::SwitchLineInStringLiteral);
                }
                needEndString = false;
                needEndChar = false;
                isEscaping = false;
            }

            if (tokenBuffer.empty())
            {
                tokenStart = {sourceOffsets.empty() ? offset : sourceOffsets[offset], line, column + 1};
            }
            tokenBuffer += currentChar;
            ++column;

            // The first code unit is already in tokenBuffer.
            const auto appendRest = [&](std::size_t length) {
                tokenBuffer.append(source.substr(offset + 1, length - 1));
                offset += length - 1;
                column += length - 1;
            };

            if (newLine)
            {
                if (currentChar == u'\r' && nextChar == u'\n')
                {
                    appendRest(2);
                }
                if (!needEndComment)
                {
                    if (spaceAndLine)
                    {
                        pushToken(TokenType::NewLine);
                    }
                    else
                    {
                        tokenBuffer.clear();
                    }
                }
                ++line;
                column = 0;
                continue;
            }

            if (needEndString || needEndChar)
            {
                if (isEscaping)
                {
                    if (currentChar >= u'0' && currentChar <= u'7')
                    {
                        const std::size_t limit = currentChar <= u'3' ? 3 : 2;
                        std::size_t length = 1;
                        while (length < limit && offset + length < sourceSize
                            && source[offset + length] >= u'0' && source[offset + length] <= u'7')
                        {
                            ++length;
                        }
                        appendRest(length);
                    }
                    else if (std::u16string_view(u"btnfr\"'\\").find(currentChar) == std::u16string_view::npos)
                    {
                        invalidEscape = true;
                    }
                    isEscaping = false;
                    ++literalLength;
                    continue;
                }
                if (currentChar == u'\\')
                {
                    isEscaping = true;
                    continue;
                }
                if (currentChar == (needEndChar ? u'\'' : u'"'))
                {
                    if (invalidEscape)
                    {
                        pushInvalid(InvalidTokenType::InvalidEscapeSequence);
                    }
                    else if (needEndChar && literalLength != 1)
                    {
                        pushInvalid(literalLength == 0 ? InvalidTokenType::EmptyCharacterLiteral
                                                       : InvalidTokenType::TooLargeCharacterLiteral);
                    }
                    else
                    {
                        pushToken(needEndChar ? TokenType::CharacterLiteral : TokenType::StringLiteral);
                    }
                    needEndString = false;
                    needEndChar = false;
                    continue;
                }
                ++literalLength;
                continue;
            }

            if (needEndComment)
            {
                if (needEndCommentType != TokenType::LineComment
                    && tokenBuffer.size() >= 4 && tokenBuffer.ends_with(u"*/"))
                {
                    finishComment();
                }
                continue;
            }

            if (isWhitespace(currentChar))
            {
                std::size_t end = offset + 1;
                while (end < sourceSize && isWhitespace(source[end]))
                {
                    ++end;
                }
                appendRest(end - offset);
                if (spaceAndLine)
                {
                    pushToken(TokenType::Whitespace);
                }
                else
                {
                    tokenBuffer.clear();
                }
                continue;
            }

            if (currentChar == u'"' || currentChar == u'\'')
            {
                needEndString = currentChar == u'"';
                needEndChar = currentChar == u'\'';
                literalLength = 0;
                invalidEscape = false;
                continue;
            }

            if (currentChar == u'/' && nextChar == u'/')
            {
                appendRest(2);
                needEndComment = true;
                needEndCommentType = TokenType::LineComment;
                continue;
            }

            if (currentChar == u'/' && nextChar == u'*')
            {
                needEndComment = true;
                needEndCommentType = offset + 2 < sourceSize && source[offset + 2] == u'*'
                    ? TokenType::JavadocComment : TokenType::BlockComment;
                appendRest(2);
                continue;
            }

            const auto remaining = source.substr(offset);
            if (isDecimalDigit(currentChar) || (currentChar == u'.' && isDecimalDigit(nextChar)))
            {
                const bool hex = currentChar == u'0' && (nextChar == u'x' || nextChar == u'X');
                const bool binary = currentChar == u'0' && (nextChar == u'b' || nextChar == u'B');
                std::size_t length = hex || binary ? 2 : 0;
                const auto scanDigits = [&](bool useHex) {
                    while (length < remaining.size() && (remaining[length] == u'_'
                        || (useHex ? isHexDigit(remaining[length]) : isDecimalDigit(remaining[length]))))
                    {
                        ++length;
                    }
                };

                scanDigits(hex);
                if (!binary && length < remaining.size() && remaining[length] == u'.')
                {
                    ++length;
                    scanDigits(hex);
                }

                if (length < remaining.size() && !binary
                    && (hex ? (remaining[length] == u'p' || remaining[length] == u'P')
                            : (remaining[length] == u'e' || remaining[length] == u'E')))
                {
                    ++length;
                    if (length < remaining.size() && (remaining[length] == u'+' || remaining[length] == u'-'))
                    {
                        ++length;
                    }
                    scanDigits(false);
                }

                if (length < remaining.size())
                {
                    const char16_t suffix = remaining[length];
                    if (suffix == u'l' || suffix == u'L'
                        || (!binary && std::u16string_view(u"fFdD").find(suffix) != std::u16string_view::npos))
                    {
                        ++length;
                    }
                }

                const auto spelling = remaining.substr(0, length);
                const bool integer = jcutils::isInteger(spelling);
                const bool floating = !integer && jcutils::isFloat(spelling);
                const auto wordLength = numericStart ? getJavaIdentifierLength(remaining, true) : 0;

                if (wordLength > length || (wordLength == length && !integer && !floating))
                {
                    appendRest(wordLength);
                    pushToken(TokenType::Identifier, std::nullopt, std::nullopt, std::nullopt, true);
                }
                else
                {
                    appendRest(length);
                    if (integer || floating)
                    {
                        pushToken(integer ? TokenType::IntegerLiteral : TokenType::FloatLiteral);
                    }
                    else
                    {
                        pushInvalid(InvalidTokenType::InvalidNumericLiteral);
                    }
                }
                continue;
            }

            if (const auto length = getJavaIdentifierLength(remaining, numericStart))
            {
                appendRest(length);
                const auto* keyword = findKeywordUtf16(tokenBuffer);
                if (!keyword || !keyword->isSupported(sourceLangLevel))
                {
                    pushToken(TokenType::Identifier, std::nullopt, std::nullopt, std::nullopt, true);
                    continue;
                }

                switch (keyword->getKind())
                {
                    case JavaKeywordKind::Reserved:
                    case JavaKeywordKind::ReservedUnused:
                        pushToken(TokenType::Keyword, std::nullopt, *keyword, std::nullopt, keywordIdentifiers);
                        break;
                    case JavaKeywordKind::Contextual:
                        pushToken(TokenType::Identifier, std::nullopt, *keyword, std::nullopt, true);
                        break;
                    case JavaKeywordKind::BooleanLiteral:
                        pushToken(TokenType::BooleanLiteral, std::nullopt, *keyword);
                        break;
                    case JavaKeywordKind::NullLiteral:
                        pushToken(TokenType::NullLiteral, std::nullopt, *keyword);
                        break;
                }
                continue;
            }

            constexpr std::size_t maxSymbolLength = 4; // >>>=
            const auto symbolText = remaining.substr(0, maxSymbolLength);
            std::size_t length = symbolText.size();
            std::optional<SymbolType> symbol;
            for (; length > 0; --length)
            {
                symbol = findSymbol(symbolText.substr(0, length));
                if (symbol)
                {
                    break;
                }
            }
            if (symbol)
            {
                appendRest(length);
                pushToken(TokenType::Symbol, symbol);
            }
            else
            {
                pushInvalid(InvalidTokenType::UnknownTokenType);
            }
        }

        if (needEndComment)
        {
            if (needEndCommentType == TokenType::LineComment)
            {
                finishComment();
            }
            else
            {
                pushInvalid(InvalidTokenType::NoneFinishMultiLineComment);
            }
        }
        if (needEndString || needEndChar)
        {
            if (isEscaping)
            {
                pushInvalid(needEndChar ? InvalidTokenType::NoneFinishEscapeInCharacterLiteral
                                        : InvalidTokenType::NoneFinishEscapeInStringLiteral);
            }
            else
            {
                pushInvalid(needEndChar ? InvalidTokenType::NoneFinishCharacterLiteral
                                        : InvalidTokenType::NoneFinishStringLiteral);
            }
        }

        tokenStart = {sourceCode.size(), line, column + 1};
        pushToken(TokenType::EndOfFile);

        return {std::move(tokens), std::move(invalidTokens)};
    }
} // javac
