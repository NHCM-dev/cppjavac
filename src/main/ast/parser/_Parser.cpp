//
// Created by NHCM on 2026/11/24.
//

#include "Parser.h"

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

    const Token& Parser::peek(std::size_t offset) const
    {
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
        return tokens().at(tokenPointer++);
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

    bool Parser::checkKeyword(const JavaKeyword& keyword) const
    {
        return peek().keyword.has_value() && peek().keyword.value() == keyword;
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

    const Token& Parser::expectIdentifier()
    {
        if (peek().canBeIdentifier)
        {
            return advance();
        }
        throw parser_exception(ParserExceptionType::ExpectIdentifier, peek());
    }

}