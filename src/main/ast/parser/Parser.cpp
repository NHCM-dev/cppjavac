//
// Created by NHCM on 2026/11/23.
//

#include "Parser.h"

#include <utility>

namespace javac
{
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
        return std::move(compilationUnit);
    }

    void Parser::parseOrdinary()
    {
        try
        {

        } catch (parser_exception)
        {

        }
        PackageDecl packageDecl = parsePackageDecl();
    }

    void Parser::parseModular()
    {

    }

    PackageDecl Parser::parsePackageDecl(std::vector<Annotation> annotations)
    {
        expectKeyword(PACKAGE);

        PackageDecl decl;
        decl.annotations = std::move(annotations);
        decl.name = expectIdentifier().content;

        while (matchSymbol(SymbolType::Dot))
        {
            decl.name += u'.';
            decl.name += expectIdentifier().content;
        }

        expectSymbol(SymbolType::Semicolon);
        return decl;
    }

    ClassDecl Parser::parseClassDecl()
    {

    }
    MethodDecl Parser::parseMethodDecl()
    {

    }
    Annotation Parser::parseAnnotation()
    {
        expectSymbol(SymbolType::AtSign);

        Annotation annotation;

        annotation.name = expectIdentifier().content;
        while (matchSymbol(SymbolType::Dot))
        {
            annotation.name += u'.';
            annotation.name += expectIdentifier().content;
        }
    }

    ParenExpr parseParenthesis()
    {

    }
} // javac
