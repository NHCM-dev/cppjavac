//
// Created by NHCM on 2026/11/23.
//

#include "Parser.h"

#include <utility>

#include "ParserException.h"
#include "ParserUtil.h"

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

    ParserResult Parser::parse()
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
        ParserResult result;
        result.compilationUnit = std::move(compilationUnit);
        //TODO: IMPORTANT: result.exceptions has to be collected,
        // the current design of Parser is that when any exception thrown, the entire process will terminate!
        return result;
    }

    void Parser::parseOrdinary()
    {
        std::vector<Annotation> annotations = parseAnnotations();
        if (checkKeyword(PACKAGE))
        {
            compilationUnit.packageDecl = parsePackageDecl(std::move(annotations));
            annotations = parseAnnotations();
        }

        while (checkKeyword(IMPORT))
        {
            compilationUnit.importDecls.push_back(parseImportDecl());
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
                modifier.range = annotation.range;
                modifier.annotation = std::move(annotation);
                modifiers.push_back(std::move(modifier));
            }
            for (auto& modifier : parseModifierFlags())
            {
                modifiers.push_back(std::move(modifier));
            }

            Uptr<ClassDecl> classDecl = makeUniq<ClassDecl>(parseClassDecl(std::move(modifiers)));
            compilationUnit.decls.push_back(std::move(classDecl));
            annotations = parseAnnotations();
        }
    }

    void Parser::parseModular()
    {
        //TODO: implement
    }

    TypeNode Parser::parseTypeNode(std::vector<Annotation> leadingAnnotations,
        std::optional<ArrayDimension>* varargs)
    {
        TypeNode node;
        const auto finishType = [this, &node, varargs]() -> TypeNode
        {
            std::vector<ArrayDimension> dimensions = parseArrayDimension(true, varargs);
            if (dimensions.empty())
            {
                return std::move(node);
            }

            TypeNode array;
            array.objectType = ObjectType::Array;
            array.elementType = makeUniq<TypeNode>(std::move(node));
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

        if (checkKeyword())
        {
            std::optional<PrimitiveType> primitive = primitiveTypeFromKeyword(peek().keyword.value());
            if (primitive)
            {
                advance();
                return settlePrimitive(primitive.value());
            }
            if (!peek().canBeIdentifier)
            {
                throw parser_exception(ParserExceptionType::KeywordInTypeSpecification, peek());
            }
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
                namePart.typeArguments = parseTypeArgs();
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

    std::vector<ArrayDimension> Parser::parseArrayDimension(bool allowAnnotation,
        std::optional<ArrayDimension>* varargs)
    {
        std::vector<ArrayDimension> dimensions;
        while (checkSymbol(SymbolType::LBracket) || canBeAnnotation()
            || (varargs && checkSymbol(SymbolType::Ellipsis)))
        {
            if (!allowAnnotation && canBeAnnotation())
            {
                throw parser_exception(ParserExceptionType::AnnotationInArrayDimension, peek());
            }

            ArrayDimension dimension;
            dimension.annotations = parseAnnotations();
            if (varargs && matchSymbol(SymbolType::Ellipsis))
            {
                *varargs = std::move(dimension);
                break;
            }
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
                    arg.type = makeUniq<TypeNode>(parseTypeNode());
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
                    arg.type = makeUniq<TypeNode>(parseTypeNode());
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
            arg.type = makeUniq<TypeNode>(parseTypeNode(std::move(pendingAnnotations)));
            if (!jcutils::canGeneric(*arg.type))
            {
                throw parser_exception(ParserExceptionType::InvalidExactGeneric, token);
            }
            typeArgs.push_back(std::move(arg));
        } while (matchSymbol(SymbolType::Comma));
        return typeArgs;
    }

    std::vector<ModifierFlag> Parser::parseModifierFlags()
    {
        std::vector<ModifierFlag> vectors;

        bool at = canBeAnnotation();
        bool modifier = false;
        FlagType flagType;
        if (checkKeyword())
        {
            std::optional<FlagType> flag = flagTypeFromKeyword(peek().keyword.value());
            if (flag)
            {
                modifier = true;
                flagType = flag.value();
            }
        }
        while (at || modifier)
        {
            if (at)
            {
                std::vector<Annotation> annotations = parseAnnotations();
                for (Annotation& annotation : annotations)
                {
                    ModifierFlag flag;
                    flag.range = annotation.range;
                    flag.annotation = std::move(annotation);
                    vectors.push_back(std::move(flag));
                }
            } else
            {
                ModifierFlag flag;
                flag.range = peek().range;
                flag.flagType = flagType;
                vectors.push_back(std::move(flag));
                advance();
            }
            at = canBeAnnotation();
            modifier = false;
            if (checkKeyword())
            {
                std::optional<FlagType> flag = flagTypeFromKeyword(peek().keyword.value());
                if (flag)
                {
                    modifier = true;
                    flagType = flag.value();
                }
            }
        }

        return vectors;
    }

    SwitchLabel Parser::parseSwitchLabel()
    {
        SwitchLabel label;
        if (checkKeyword(DEFAULT))
        {
            label.range = expectKeyword(DEFAULT).range;
            label.hasDefault = true;
            return label;
        }

        label.range = expectKeyword(CASE).range;
        if (peek().tokenType == TokenType::NullLiteral)
        {
            if (!langLevelMeet(LangLevel::TwentyOne))
            {
                throw parser_exception(ParserExceptionType::InvalidSwitchLabel, peek());
            }
            label.caseExprs.push_back(parseConstantValue());
            label.range->end = label.caseExprs.back()->range->end;
            if (matchSymbol(SymbolType::Comma))
            {
                label.range->end = expectKeyword(DEFAULT).range.end;
                label.hasDefault = true;
            }
            return label;
        }

        do
        {
            if (peek().tokenType == TokenType::NullLiteral)
            {
                throw parser_exception(ParserExceptionType::InvalidSwitchLabel, peek());
            }
            label.caseExprs.push_back(parseConstantValue());
        } while (langLevelMeet(LangLevel::Fourteen) && matchSymbol(SymbolType::Comma));
        label.range->end = label.caseExprs.back()->range->end;
        return label;
    }

    AnnotationArg Parser::parseAnnotationArg()
    {
        AnnotationArg arg = {};
        arg.name = expectIdentifier().content;
        expectSymbol(SymbolType::Assign);
        arg.value = parseElementValue();
        return arg;
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

    CatchClause Parser::parseCatchClause()
    {
        CatchClause catchClause;

        catchClause.range = expectKeyword(CATCH).range;
        expectSymbol(SymbolType::LParen);

        catchClause.modifiers = parseModifierFlags();
        bool seenFinal = false;
        for (const auto& modifier : catchClause.modifiers)
        {
            if (!modifier.flagType)
            {
                continue;
            }
            if (modifier.flagType != FlagType::Final)
            {
                throw parser_exception(ParserExceptionType::ModifierNotAllowedForCatchParameter, modifier.range);
            }
            if (seenFinal)
            {
                throw parser_exception(ParserExceptionType::DuplicateModifier, modifier.range);
            }
            seenFinal = true;
        }

        do
        {
            const Token& start = peek();
            TypeNode type = parseTypeNode();
            if (type.objectType != ObjectType::Class)
            {
                throw parser_exception(ParserExceptionType::ExpectThrowableType, start);
            }
            catchClause.exceptionTypes.push_back(std::move(type));
        } while (matchSymbol(SymbolType::Pipe));

        catchClause.name = expectIdentifier().content;

        expectSymbol(SymbolType::RParen);

        catchClause.body = parseBlock();
        catchClause.range->end = catchClause.body->range->end;

        return catchClause;
    }
} // javac
