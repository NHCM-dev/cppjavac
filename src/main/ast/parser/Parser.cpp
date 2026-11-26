//
// Created by NHCM on 2026/11/23.
//

#include "Parser.h"

#include <utility>

namespace javac
{
    Parser::Parser(TokenStream inputStream, Path filePath, std::set<JavacExtension> extensions, LangLevel sourceLangLevel)
    : inputStream(std::move(inputStream)), filePath(std::move(filePath)),
    extensions(std::move(extensions)), sourceLangLevel(sourceLangLevel)
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
        tokenPointer = 0;
        typeCloseRemainder.reset();
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
        auto annotations = parseAnnotations();
        if (checkKeyword(PACKAGE))
        {
            compilationUnit.packageDecl = parsePackageDecl(std::move(annotations));
            annotations = parseAnnotations();
        }

        while (!isAtEnd() || !annotations.empty())
        {
            if (annotations.empty() && matchSymbol(SymbolType::Semicolon))
            {
                annotations = parseAnnotations();
                continue;
            }

            std::vector<ModifierFlag> modifiers;
            for (auto& annotation : annotations)
            {
                ModifierFlag modifier;
                modifier.annotation = std::move(annotation);
                modifiers.push_back(std::move(modifier));
            }
            compilationUnit.decls.push_back(std::make_unique<ClassDecl>(parseClassDecl(std::move(modifiers))));
            annotations = parseAnnotations();
        }
    }

    void Parser::parseModular()
    {

    }

    TypeNode Parser::parseTypeNode(std::vector<Annotation> leadingAnnotations)
    {
        TypeNode node;
        const auto finishType = [this, &node]() -> TypeNode
        {
            std::vector<ArrayDimension> dimensions = parseArrayDimension();
            if (dimensions.empty())
            {
                return std::move(node);
            }

            TypeNode array;
            array.objectType = ObjectType::Array;
            array.elementType = std::make_unique<TypeNode>(std::move(node));
            array.dimensions = std::move(dimensions);
            return array;
        };

        const auto settlePrimitive = [&node, &finishType](PrimitiveType type)
        {
            node.objectType = ObjectType::Primitive;
            node.primitiveType = type;
            return finishType();
        };

        node.annotations = leadingAnnotations.empty() ? parseAnnotations() : std::move(leadingAnnotations);

        if (matchKeyword(VOID))
        {
            node.objectType = ObjectType::Void;
            return node;
        }
        if (matchKeyword(BOOLEAN))
        {
            return settlePrimitive(PrimitiveType::Boolean);
        }
        if (matchKeyword(BYTE))
        {
            return settlePrimitive(PrimitiveType::Byte);
        }
        if (matchKeyword(SHORT))
        {
            return settlePrimitive(PrimitiveType::Short);
        }
        if (matchKeyword(CHAR))
        {
            return settlePrimitive(PrimitiveType::Char);
        }
        if (matchKeyword(INT))
        {
            return settlePrimitive(PrimitiveType::Int);
        }
        if (matchKeyword(LONG))
        {
            return settlePrimitive(PrimitiveType::Long);
        }
        if (matchKeyword(FLOAT))
        {
            return settlePrimitive(PrimitiveType::Float);
        }
        if (matchKeyword(DOUBLE))
        {
            return settlePrimitive(PrimitiveType::Double);
        }

        if (checkKeyword() && !peek().canBeIdentifier)
        {
            throw parser_exception(ParserExceptionType::KeywordInTypeSpecification, peek());
        }

        std::vector<Annotation> annotations = std::move(node.annotations);
        do
        {
            TypeNamePart namePart;
            namePart.annotations = std::move(annotations);
            namePart.name = expectIdentifier().content;

            if (matchSymbol(SymbolType::LAngleBracket))
            {
                node.objectType = ObjectType::GenericClass;
                namePart.diamond = checkSymbol(SymbolType::RAngleBracket);
                if (!namePart.diamond)
                {
                    namePart.typeArguments = parseTypeArgs();
                }
                expectTypeClose();
            }

            node.nameParts.push_back(std::move(namePart));
            if (!matchSymbol(SymbolType::Dot))
            {
                break;
            }
            annotations = parseAnnotations();
        } while (true);

        return finishType();
    }

    std::vector<ArrayDimension> Parser::parseArrayDimension(bool allowAnnotation)
    {
        std::vector<ArrayDimension> dimensions;
        while (checkSymbol(SymbolType::LBracket) || checkSymbol(SymbolType::AtSign))
        {
            if (!allowAnnotation && checkSymbol(SymbolType::AtSign))
            {
                throw parser_exception(ParserExceptionType::AnnotationInArrayDimension, peek());
            }

            ArrayDimension dimension;
            dimension.annotations = parseAnnotations();
            expectSymbol(SymbolType::LBracket);
            expectSymbol(SymbolType::RBracket);
            dimensions.push_back(std::move(dimension));
        }
        return dimensions;
    }

    std::vector<TypeArg> Parser::parseTypeArgs()
    {
        std::vector<TypeArg> typeArgs = {};
        do
        {
            TypeArg arg;
            std::vector<Annotation> pendingAnnotations = parseAnnotations();
            if (matchSymbol(SymbolType::Question))
            {
                arg.annotations = std::move(pendingAnnotations);
                if (matchKeyword(EXTENDS))
                {
                    arg.genericType = GenericType::Extends;
                    const Token& token = peek();
                    arg.type = std::make_unique<TypeNode>(parseTypeNode());
                    if (!jcutils::canGeneric(*arg.type))
                    {
                        throw parser_exception(ParserExceptionType::CannotExtendsTo, token);
                    }
                    typeArgs.push_back(std::move(arg));
                    continue;
                }
                if (matchKeyword(SUPER))
                {
                    arg.genericType = GenericType::Super;
                    const Token& token = peek();
                    arg.type = std::make_unique<TypeNode>(parseTypeNode());
                    if (!jcutils::canGeneric(*arg.type))
                    {
                        throw parser_exception(ParserExceptionType::CannotBeSuper, token);
                    }
                    typeArgs.push_back(std::move(arg));
                    continue;
                }
                arg.genericType = GenericType::Unbounded;
                typeArgs.push_back(std::move(arg));
                continue;
            }
            arg.genericType = GenericType::Exact;
            const Token& token = peek();
            arg.type = std::make_unique<TypeNode>(parseTypeNode(std::move(pendingAnnotations)));
            const bool allowPrimitive = jcutils::contains(extensions, JavacExtension::AllowPrimitiveGenerics);
            if (!jcutils::canGeneric(*arg.type, allowPrimitive))
            {
                const auto error = allowPrimitive && arg.type->objectType == ObjectType::Void
                    ? ParserExceptionType::EXTENSION_VoidInGeneric
                    : ParserExceptionType::InvalidExactGeneric;
                throw parser_exception(error, token);
            }
            typeArgs.push_back(std::move(arg));
        } while (matchSymbol(SymbolType::Comma));
        return typeArgs;
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

        if (checkSymbol(SymbolType::LAngleBracket))
        {
            std::vector<GenericParameter> generics = parseGenericParameters();
            decl.typeParameters = std::move(generics);
        }

        if (matchKeyword(EXTENDS))
        {
            const Token& token = peek();
            decl.superClass = parseTypeNode();
            if (!jcutils::canExtImplTo(*decl.superClass))
            {
                throw parser_exception(ParserExceptionType::CannotExtendsTo, token);
            }
        }
        if (matchKeyword(IMPLEMENTS))
        {
            do
            {
                const Token& token = peek();
                TypeNode type = parseTypeNode();
                if (!jcutils::canExtImplTo(type))
                {
                    throw parser_exception(ParserExceptionType::CannotImplementsTo, token);
                }
                decl.interfaces.push_back(std::move(type));
            } while (matchSymbol(SymbolType::Comma));
        }
        if (matchKeyword(PERMITS))
        {
            do
            {
                const Token& token = peek();
                TypeNode type = parseTypeNode();
                if (!jcutils::canPermit(type))
                {
                    throw parser_exception(ParserExceptionType::CannotPermitsTo, token);
                }
                decl.permittedTypes.push_back(std::move(type));
            } while (matchSymbol(SymbolType::Comma));
        }

        expectSymbol(SymbolType::LBrace);

        //TODO: parse methods and fields

        expectSymbol(SymbolType::RBrace);

        return decl;
    }


    MethodDecl Parser::parseMethodDecl()
    {

    }

    std::vector<Annotation> Parser::parseAnnotations()
    {
        std::vector<Annotation> annotations;
        while (checkSymbol(SymbolType::AtSign) && peek(1).keyword != INTERFACE)
        {
            annotations.push_back(parseAnnotation());
        }
        return annotations;
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

        if (matchSymbol(SymbolType::LParen))
        {
            if (matchSymbol(SymbolType::RParen))
            {
                return annotation;
            }
            do
            {
                AnnotationArg arg = parseAnnotationArg();
                annotation.arguments.push_back(std::move(arg));
            } while (matchSymbol(SymbolType::Comma));

            expectSymbol(SymbolType::RParen);
        }
        return annotation;
    }

    AnnotationArg Parser::parseAnnotationArg()
    {
        // TODO: Parse annotation arguments.

        AnnotationArg arg = {};
        if (checkIdentifier())
        {
            arg.name = expectIdentifier().content;
            expectSymbol(SymbolType::Assign);
        }

        arg.value = parseAnnotationValue();

        return arg;
    }

    std::unique_ptr<Expr> Parser::parseAnnotationValue()
    {
        if (matchSymbol(SymbolType::AtSign))
        {
            return std::make_unique<Annotation>(parseAnnotation());
        }

        if (matchSymbol(SymbolType::LBrace))
        {
            return parseAnnotationArray();
        }

        return parseAnnotationExpr();
    }

    std::unique_ptr<Expr> Parser::parseAnnotationArray()
    {
        //TODO: implement array value
        expectSymbol(SymbolType::RBrace);
    }

    std::unique_ptr<Expr> Parser::parseAnnotationExpr()
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
                    const Token& token = peek();
                    TypeNode type = parseTypeNode();
                    if (!jcutils::canExtImplTo(type))
                    {
                        throw parser_exception(ParserExceptionType::CannotExtendsTo, token);
                    }
                    parameter.bounds.push_back(std::move(type));
                }
                while (matchSymbol(SymbolType::Ampersand));
            }

            parameters.push_back(std::move(parameter));
        }
        while (matchSymbol(SymbolType::Comma));

        expectTypeClose();
        return parameters;
    }

    std::unique_ptr<Expr> Parser::parseBasicLiteral()
    {
        const Token& token = advance();
        switch (token.tokenType)
        {
            case TokenType::IntegerLiteral:
            {
                jlong number;
                try
                {
                    number = jcutils::int64FromString(token.content);
                } catch (const std::invalid_argument& e)
                {
                    throw parser_exception(ParserExceptionType::InvalidNumberLiteral, token, e.what());
                }
                catch (const std::out_of_range& e)
                {
                    throw parser_exception(ParserExceptionType::OutOfRangeNumberLiteral, token, e.what());
                }

                if (jcutils::int64IsLong(number, token.content))
                {
                    LongExpr longExpr;
                    longExpr.value = number;
                    return  std::make_unique<LongExpr>(longExpr);
                }
                IntExpr intExpr;
                intExpr.value = static_cast<int32_t>(number);
                return  std::make_unique<IntExpr>(intExpr);
            }
            case TokenType::FloatLiteral:
            {
                try
                {
                    if (jcutils::is32Float(token.content))
                    {
                        FloatExpr floatExpr;
                        floatExpr.value = jcutils::floatFromString(token.content);
                        return std::make_unique<FloatExpr>(floatExpr);
                    }

                    DoubleExpr doubleExpr;
                    doubleExpr.value = jcutils::doubleFromString(token.content);
                    return std::make_unique<DoubleExpr>(doubleExpr);
                }
                catch (const std::invalid_argument& e)
                {
                    throw parser_exception(ParserExceptionType::InvalidNumberLiteral, token, e.what());
                }
                catch (const std::out_of_range& e)
                {
                    throw parser_exception(ParserExceptionType::OutOfRangeNumberLiteral, token, e.what());
                }
            }
            case TokenType::StringLiteral:
            {
                StringExpr stringExpr;
                stringExpr.value = peek().content;
                return  std::make_unique<StringExpr>(stringExpr);
            }
            case TokenType::CharacterLiteral:
            {
                CharExpr charExpr;
                charExpr.value = peek().content.at(0);
                return  std::make_unique<CharExpr>(charExpr);
            }
            case TokenType::BooleanLiteral:
            {
                BooleanExpr boolExpr;
                boolExpr.value = jcutils::boolFromString(token.content);
                return std::make_unique<BooleanExpr>(boolExpr);
            }
            case TokenType::NullLiteral:
            {
                NullExpr nullExpr;
                return std::make_unique<NullExpr>(nullExpr);
            }
            default:
            {
                throw parser_exception(ParserExceptionType::NotLiteral, token);
            }
        }
    }

    std::unique_ptr<ClassLiteralExpr> Parser::parseClassLiteral()
    {
        TypeNode node;

        const auto finishType = [this, &node]
        {
            if (node.objectType != ObjectType::Void)
            {
                auto dimensions = parseArrayDimension(false);
                if (!dimensions.empty())
                {
                    TypeNode array;
                    array.objectType = ObjectType::Array;
                    array.elementType = std::make_unique<TypeNode>(std::move(node));
                    array.dimensions = std::move(dimensions);
                    node = std::move(array);
                }
            }

            expectSymbol(SymbolType::Dot);
            expectKeyword(CLASS);
            auto literal = std::make_unique<ClassLiteralExpr>();
            literal->type = std::move(node);
            return literal;
        };

        const auto settlePrimitive = [&node](PrimitiveType type)
        {
            node.objectType = ObjectType::Primitive;
            node.primitiveType = type;
        };

        if (matchKeyword(VOID))
        {
            node.objectType = ObjectType::Void;
            return finishType();
        }
        if (matchKeyword(BOOLEAN))
        {
            settlePrimitive(PrimitiveType::Boolean);
            return finishType();
        }
        if (matchKeyword(BYTE))
        {
            settlePrimitive(PrimitiveType::Byte);
            return finishType();
        }
        if (matchKeyword(SHORT))
        {
            settlePrimitive(PrimitiveType::Short);
            return finishType();
        }
        if (matchKeyword(CHAR))
        {
            settlePrimitive(PrimitiveType::Char);
            return finishType();
        }
        if (matchKeyword(INT))
        {
            settlePrimitive(PrimitiveType::Int);
            return finishType();
        }
        if (matchKeyword(LONG))
        {
            settlePrimitive(PrimitiveType::Long);
            return finishType();
        }
        if (matchKeyword(FLOAT))
        {
            settlePrimitive(PrimitiveType::Float);
            return finishType();
        }
        if (matchKeyword(DOUBLE))
        {
            settlePrimitive(PrimitiveType::Double);
            return finishType();
        }

        while (true)
        {
            if (checkKeyword() && !checkIdentifier())
            {
                throw parser_exception(ParserExceptionType::KeywordInClassLiteral, peek());
            }

            TypeNamePart namePart;
            const auto& name = expectIdentifier();
            namePart.name = name.content;

            if (checkSymbol(SymbolType::LAngleBracket))
            {
                throw parser_exception(ParserExceptionType::GenericInClassLiteral, peek());
            }

            node.nameParts.push_back(std::move(namePart));
            if (!checkSymbol(SymbolType::Dot) || peek(1).keyword == CLASS)
            {
                const auto* keyword = findKeywordUtf16(name.content);
                if (keyword && keyword->isSupported(sourceLangLevel)
                    && (*keyword == VAR || *keyword == YIELD || *keyword == RECORD
                        || *keyword == SEALED || *keyword == PERMITS))
                {
                    throw parser_exception(ParserExceptionType::KeywordInClassLiteral, name);
                }
                break;
            }
            advance();
        }

        return finishType();
    }
} // javac
