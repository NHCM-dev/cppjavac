//
// Created by NHCM on 2026/11/24.
//

#include "Parser.h"

#include <algorithm>

#include "ParserException.h"

namespace javac
{
    void Parser::errorProcessPlaceholder()
    {

    }

    CompilationUnitType Parser::decideType(const Path& filePath)
    {
        std::string fileName = filePath.filename().string();
        if (fileName == "module-info.java")
        {
            return CompilationUnitType::Modular;
        }
        return CompilationUnitType::Ordinary;
    }

    const std::vector<Token>& Parser::tokens() const
    {
        return this->inputStream.tokens;
    }

    bool Parser::extensionEnabled(JavacExtension extension) const
    {
        return jcutils::contains(extensions, extension);
    }

    bool Parser::canBeAnnotation() const
    {
        return checkSymbol(SymbolType::AtSign) && peek(1).keyword != INTERFACE;
    }

    bool Parser::langLevelMeet(LangLevel targetLangLevel) const
    {
        return static_cast<uint8_t>(sourceLangLevel) >= static_cast<uint8_t>(targetLangLevel);
    }

    const Token& Parser::peek(std::size_t offset) const
    {
        if (offset == 0 && typeCloseRemainder && typeCloseRemainder->first == tokenPointer)
        {
            return typeCloseRemainder->second;
        }
        if (tokenPointer + offset >= tokens().size())
        {
            offset = 0;
        }
        return tokens().at(tokenPointer + offset);
    }
    const Token& Parser::advance()
    {
        if (tokenPointer + 1 >= tokens().size())
        {
            return tokens().back();
        }
        const auto& token = peek();
        ++tokenPointer;
        return token;
    }
    bool Parser::isAtEnd() const
    {
        if (tokenPointer >= tokens().size())
        {
            return true;
        }
        return peek().tokenType == TokenType::EndOfFile;
    }

    bool Parser::checkSymbol(SymbolType type) const
    {
        return peek().symbol.has_value() && peek().symbol.value() == type;
    }
    bool Parser::matchSymbol(SymbolType type)
    {
        if (checkSymbol(type))
        {
            advance();
            return true;
        }
        return false;
    }

    const Token& Parser::expectSymbol(SymbolType type)
    {
        if (checkSymbol(type))
        {
            return advance();
        }
        throw parser_exception(ParserExceptionType::ExpectSymbol, peek());
    }

    void Parser::expectTypeClose()
    {
        if (matchSymbol(SymbolType::RAngleBracket))
        {
            return;
        }

        const Token& token = peek();
        if (token.symbol && token.content.size() > 1 && token.content.front() == u'>')
        {
            if (const auto symbol = findSymbol(std::u16string_view(token.content).substr(1)))
            {
                auto remainder = token;
                remainder.content.erase(0, 1);
                remainder.symbol = symbol;
                auto& begin = remainder.range.begin;
                ++begin.column;
                const auto& offsets = inputStream.sourceOffsets;
                if (offsets.empty())
                {
                    ++begin.offsetInFile;
                }
                else
                {
                    begin.offsetInFile = *std::upper_bound(offsets.begin(), offsets.end(), begin.offsetInFile);
                }
                typeCloseRemainder.emplace(tokenPointer, std::move(remainder));
                return;
            }
        }

        throw parser_exception(ParserExceptionType::ExpectSymbol, token, "Expected '>'");
    }

    bool Parser::checkKeyword() const
    {
        return peek().keyword.has_value();
    }

    bool Parser::checkKeyword(const JavaKeyword& keyword) const
    {
        return checkKeyword() && peek().keyword.value() == keyword;
    }

    bool Parser::matchKeyword(const JavaKeyword& keyword)
    {
        if (checkKeyword(keyword))
        {
            advance();
            return true;
        }
        return false;
    }

    const Token& Parser::expectKeyword(const JavaKeyword& keyword)
    {
        if (checkKeyword(keyword))
        {
            return advance();
        }
        throw parser_exception(ParserExceptionType::ExpectKeyword, peek());
    }

    bool Parser::checkIdentifier() const
    {
        return peek().canBeIdentifier;
    }

    bool Parser::matchIdentifier()
    {
        if (checkIdentifier())
        {
            advance();
            return true;
        }
        return false;
    }

    const Token& Parser::expectIdentifier()
    {
        if (checkIdentifier())
        {
            return advance();
        }
        throw parser_exception(ParserExceptionType::ExpectIdentifier, peek());
    }
}
