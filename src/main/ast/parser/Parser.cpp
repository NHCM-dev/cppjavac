//
// Created by NHCM on 2026/11/23.
//

#include "Parser.h"

namespace javac
{
    void Parser::parseModular()
    {
        compilationUnit.unitType = CompilationUnitType::Modular;
    }

    void Parser::parseOrdinary()
    {
        compilationUnit.unitType = CompilationUnitType::Ordinary;

        for (const auto& token : inputStream.tokens)
        {
            switch (token.tokenType)
            {
                case TokenType::Whitespace:
                case TokenType::NewLine:
                {
                    processWhitespace();
                    break;
                }
                case TokenType::Keyword:
                {
                    processKeywords(token);
                    break;
                }
                case TokenType::Identifier:
                {
                    processIdentifier(token);
                    break;
                }
                case TokenType::IntegerLiteral:
                {
                    break;
                }
                case TokenType::FloatLiteral:
                {
                    break;
                }
                case TokenType::CharacterLiteral:
                {
                    break;
                }
                case TokenType::StringLiteral:
                {
                    break;
                }
                case TokenType::Symbol:
                {
                    processSymbol(token);
                    break;
                }

                case TokenType::BooleanLiteral:
                {
                    break;
                }
                case TokenType::NullLiteral:
                {
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

    }

    void Parser::processWhitespace()
    {
        // Value pending
        // if (isOp(ParserOp::PendingAnnotation))
        // {
        //     //TODO: finish annotation collecting
        //     pushAnnotation({});
        //     clearOp();
        //     return;
        // }

        if (isOp(ParserOp::PendingPackage) && !readBuffer(S_PACKAGE).empty())
        {
            // prevent reading space characters into package declaration
            appendBuffer(S_PACKAGE, u' ');
            return;
        }
    }

    void Parser::processKeywords(const Token& token)
    {
        JavaKeyword keyword = token.keyword.value();

        // package
        if (keyword == PACKAGE)
        {
            if (opUnfinished())
            {
                //TODO: package requires clean start, error handle here
                errorProcessPlaceholder();
                return;
            }
            if (!getOrCreatePackage().name.empty())
            {
                //TODO: Record error of multiple package declaration
                errorProcessPlaceholder();
                return;
            }
            setOp(ParserOp::PendingPackage);
            return;
        }


    }

    void Parser::processIdentifier(const Token& token)
    {
        std::u16string_view content = token.content;

        // Annotation @XXX
        if (isOp(ParserOp::PendingAnnotation))
        {
            appendBuffer(S_ANNOTATION, content);
            return;
        }

        // package XXX
        if (isOp(ParserOp::PendingPackage))
        {
            appendBuffer(S_PACKAGE, content);
            return;
        }
    }

    void Parser::processSymbol(const Token& token)
    {
        SymbolType symbolType = token.symbol.value();
        char16_t content = u'.';

        // @
        if (symbolType == SymbolType::AtSign)
        {
            if (opUnfinished())
            {
                //TODO: annotation starting check, error handle here
                errorProcessPlaceholder();
                return;
            }
            setOp(ParserOp::PendingAnnotation);
            return;
        }

        // .
        if (symbolType == SymbolType::Dot)
        {
            if (isOp(ParserOp::PendingAnnotation))
            {
                appendBuffer(S_ANNOTATION, content);
                return;
            }
            if (isOp(ParserOp::PendingPackage))
            {
                appendBuffer(S_PACKAGE, content);
                return;
            }
            return;
        }

        // ;
        if (symbolType == SymbolType::Semicolon)
        {
            if (isOp(ParserOp::PendingAnnotation))
            {
                //TODO: Cannot put semicolon after annotation
                errorProcessPlaceholder();
                return;
            }
            if (isOp(ParserOp::PendingPackage))
            {
                PackageDecl& packDecl = getOrCreatePackage();
                packDecl.annotations.clear();
                std::queue<Annotation> annotations = flushAnnotation();
                while (!annotations.empty())
                {
                    packDecl.annotations.push_back(std::move(annotations.front()));
                    annotations.pop();
                }
                packDecl.name = flushBuffer(S_PACKAGE);
                clearOp();
                if (jcutils::hasSpace(packDecl.name))
                {
                    //TODO: cannot put any space in package name
                    errorProcessPlaceholder();
                    return;
                }
                return;
            }
            return;
        }

        // (
        if (symbolType == SymbolType::LParen)
        {
            if (isOp(ParserOp::PendingAnnotation))
            {
                pushDelimiter(SymbolType::LParen);
            }
        }
    }

    Parser::Parser(TokenStream inputStream, Path filePath)
       : inputStream(std::move(inputStream)), filePath(std::move(filePath))
    {
        if (!this->inputStream.invalidTokens.empty())
        {
            throw std::invalid_argument("Token stream contains invalid tokens");
        }
        jcutils::removeIf(this->inputStream.tokens, [](const Token& token)
        {
            return token.tokenType == TokenType::LineComment
            || token.tokenType == TokenType::BlockComment
            || token.tokenType == TokenType::JavadocComment;
        });
    }

    CompilationUnit Parser::parse()
    {
        compilationUnit = {};
        CompilationUnitType compilationUnitType = decideType(this->filePath);

        op = ParserOp::None;
        stringBufferSlots = {};
        clearDelimiters();

        getOrCreatePackage() = {};

        switch (compilationUnitType)
        {
            case CompilationUnitType::Modular:
            {
                parseModular();
                break;
            }
            default:
            {
                parseOrdinary();
                break;
            }
        }
        if (!delimiterOrder.empty())
        {
            errorProcessPlaceholder();
        }
        return std::move(compilationUnit);
    }
} // javac
