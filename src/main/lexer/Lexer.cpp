//
// Created by NHCM on 2026/11/22.
//

#include "Lexer.h"

#include <algorithm>
#include <utility>

#include "main/util/MathUtil.h"

namespace javac
{
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

    std::span<const char16_t> Lexer::readNextX(std::size_t startOffset, std::size_t length)
    {
        const std::size_t sourceSize = sourceCode.size();

        if (startOffset >= sourceSize)
        {
            return {};
        }

        const std::size_t amountToRead = std::min(length, sourceSize - startOffset);

        return {sourceCode.data() + startOffset, amountToRead};
    }

    std::vector<Token> Lexer::lex(bool spaceAndLine, bool comment)
    {
        tokens.clear();
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
        const std::size_t sourceSize = sourceCode.size();
        const std::u16string_view source = sourceCode;

        const auto isDecimalDigit = [](char16_t ch) { return ch >= u'0' && ch <= u'9'; };
        const auto isHexDigit = [&](char16_t ch) {
            return isDecimalDigit(ch) || (ch >= u'a' && ch <= u'f') || (ch >= u'A' && ch <= u'F');
        };
        const auto isWhitespace = [](char16_t ch) { return ch == u' ' || ch == u'\t' || ch == u'\f'; };
        const auto pushInvalid = [&](InvalidTokenType reason) {
            pushToken(TokenType::Invalid, std::nullopt, std::nullopt, reason);
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
            const char16_t currentChar = sourceCode[offset];
            const char16_t nextChar = offset + 1 < sourceSize ? sourceCode[offset + 1] : u'\0';
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
                tokenStart = {offset, line, column + 1};
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
                            && sourceCode[offset + length] >= u'0' && sourceCode[offset + length] <= u'7')
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
                while (end < sourceSize && isWhitespace(sourceCode[end]))
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
                needEndCommentType = offset + 2 < sourceSize && sourceCode[offset + 2] == u'*'
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
            const auto symbolText = readNextX(offset, maxSymbolLength);
            std::size_t length = symbolText.size();
            std::optional<SymbolType> symbol;
            for (; length > 0; --length)
            {
                symbol = findSymbol(std::u16string_view(symbolText.data(), length));
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

        tokenStart = {offset, line, column + 1};
        pushToken(TokenType::EndOfFile);

        return std::move(tokens);
    }
} // javac
