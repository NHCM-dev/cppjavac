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

    ClassDecl Parser::parseClassDecl(std::vector<ModifierFlag> modifiers)
    {
        expectKeyword(CLASS);

        ClassDecl decl;
        decl.modifiers = std::move(modifiers);
        decl.name = expectIdentifier().content;

        if (matchSymbol(SymbolType::LAngleBracket))
        {
            std::vector<GenericParameter> generics = parseGenericParameters();
            decl.typeParameters = std::move(generics);
        }

        

        return decl;
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

        //TODO: parse annotation arg
    }

    AnnotationArg Parser::parseAnnotationArg()
    {

    }

    std::vector<GenericParameter> Parser::parseGenericParameters()
    {
        expectSymbol(SymbolType::LAngleBracket);

        std::vector<GenericParameter> parameters;

        do
        {
            GenericParameter parameter;
            parameter.name = expectIdentifier().content;

            if (matchKeyword(EXTENDS))
            {
                do
                {
                    parameter.bounds.push_back(parseTypeNode());
                }
                while (matchSymbol(SymbolType::Ampersand));
            }

            parameters.push_back(std::move(parameter));
        }
        while (matchSymbol(SymbolType::Comma));

        expectSymbol(SymbolType::RAngleBracket);
        return parameters;
    }

    TypeNode Parser::parseTypeNode()
    {

    }
} // javac
