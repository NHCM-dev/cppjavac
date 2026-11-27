//
// Created by NHCM on 2026/11/23.
//

#include "Parser.h"

#include <algorithm>
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

            std::unique_ptr<ClassDecl> classDecl = std::make_unique<ClassDecl>(parseClassDecl(std::move(modifiers)));
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
            if (!jcutils::canGeneric(*arg.type))
            {
                throw parser_exception(ParserExceptionType::InvalidExactGeneric, token);
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

    ClassDecl Parser::parseClassDecl(std::vector<ModifierFlag> modifiers, jcutils::ClassDeclContext context)
    {
        expectKeyword(CLASS);

        ClassDecl classDecl;

        if (const auto* invalid = jcutils::findInvalidClassModifier(modifiers, sourceLangLevel, context))
        {
            throw parser_exception(ParserExceptionType::ModifierNotAllowedForClass, invalid->range);
        }
        if (const auto conflict = jcutils::findModifierConflict(modifiers, jcutils::ModifierDeclaration::Class))
        {
            throw parser_exception(conflict->exception, conflict->range);
        }
        classDecl.modifiers = std::move(modifiers);
        classDecl.name = expectIdentifier().content;

        if (checkSymbol(SymbolType::LAngleBracket))
        {
            std::vector<GenericParameter> generics = parseGenericParameters();
            classDecl.typeParameters = std::move(generics);
        }

        if (matchKeyword(EXTENDS))
        {
            const Token& token = peek();
            classDecl.superClass = parseTypeNode();
            if (!jcutils::canExtImplTo(*classDecl.superClass))
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
                classDecl.interfaces.push_back(std::move(type));
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
                classDecl.permittedTypes.push_back(std::move(type));
            } while (matchSymbol(SymbolType::Comma));
        }

        expectSymbol(SymbolType::LBrace);

        // A normal class has its own class body even when declared inside an interface.
        const jcutils::ClassDeclContext bodyContext =
            jcutils::isInterfaceBody(context) ? jcutils::ClassDeclContext::Member : context;
        const auto methodContext = jcutils::haveModifier(classDecl.modifiers, FlagType::Abstract)
            ? jcutils::MethodDeclContext::WithinAbstractClass : jcutils::MethodDeclContext::Normal;

        while (std::unique_ptr<Decl> member = parseNormalClassMember(classDecl.name, bodyContext, methodContext))
        {
            classDecl.members.push_back(std::move(member));
        }

        expectSymbol(SymbolType::RBrace);

        return classDecl;
    }

    std::unique_ptr<Decl> Parser::parseNormalClassMember(std::u16string_view className,
        jcutils::ClassDeclContext classContext, jcutils::MethodDeclContext methodContext,
        std::optional<ReceiverParameter> receiverParameter)
    {
        while (matchSymbol(SymbolType::Semicolon)){}

        // Nothing passed
        if (checkSymbol(SymbolType::RBrace))
        {
            return nullptr;
        }
        
        // Try parse initializer such as static block
        bool isStatic = checkKeyword(STATIC) && peek(1).symbol == SymbolType::LBrace;
        if (isStatic)
        {
            if (classContext != jcutils::ClassDeclContext::TopLevel)
            {
                throw parser_exception(ParserExceptionType::StaticInitializerInMemberClass, peek());
            }
            return parseInitializerDecl();
        }
        if (checkSymbol(SymbolType::LBrace))
        {
            return parseInitializerDecl();
        }

        // Collect annotations
        // Parse modifier flags
        std::vector<ModifierFlag> modifiers = parseModifierFlags();

        // Check for class
        if (checkKeyword(CLASS))
        {
            return std::make_unique<ClassDecl>(parseClassDecl(std::move(modifiers), jcutils::ClassDeclContext::Member));
        }
        //TODO: Check for interface
        //TODO: Check for enum
        //TODO: Check for record (Java17+)
        if (checkKeyword(INTERFACE) || checkKeyword(ENUM)
            || (checkSymbol(SymbolType::AtSign) && peek(1).keyword == INTERFACE)
            || (langLevelMeet(LangLevel::Seventeen) && checkKeyword(RECORD)))
        {
            //TODO: Parse nested interfaces, annotation types, enums and records.
            throw parser_exception(ParserExceptionType::UnsupportedMemberTypeDeclaration, peek());
        }

        std::vector<GenericParameter> typeParameters;
        const Token& prefixStart = peek();
        if (checkSymbol(SymbolType::LAngleBracket))
        {
            typeParameters = parseGenericParameters();
        }
        // Check for constructor
        if (checkIdentifier() && peek(1).symbol == SymbolType::LParen)
        {
            const Token& name = expectIdentifier();
            if (name.content != className)
            {
                throw parser_exception(ParserExceptionType::ConstructorNameMismatch, name);
            }
            return std::make_unique<ConstructorDecl>(parseConstructorDecl(std::move(modifiers), classContext,
                std::move(typeParameters), name, std::move(receiverParameter)));
        }

        // Check for field and method
        // parseTypeNode() handles primitive and reference types.
        const Token& typeStart = peek();
        TypeNode type = parseTypeNode();
        const Token& name = expectIdentifier();

        // '(' means is a method
        if (checkSymbol(SymbolType::LParen))
        {
            return std::make_unique<MethodDecl>(
                parseMethodDecl(std::move(modifiers),
                    methodContext,
                    std::move(typeParameters),
                    std::move(type),
                    typeStart,
                    name,
                    std::move(receiverParameter)));
        }

        if (!checkSymbol(SymbolType::LBracket)
            && !canBeAnnotation()
            && !checkSymbol(SymbolType::Assign)
            && !checkSymbol(SymbolType::Comma)
            && !checkSymbol(SymbolType::Semicolon))
        {
            throw parser_exception(ParserExceptionType::InvalidClassMember, peek());
        }

        // <T> void method(), field declaration cannot start with <
        if (!typeParameters.empty())
        {
            throw parser_exception(ParserExceptionType::TypeParametersNotAllowedForField, prefixStart);
        }
        // void method(), field declaration cannot have type void
        if (type.objectType == ObjectType::Void)
        {
            throw parser_exception(ParserExceptionType::VoidAsTypeSpecification, typeStart);
        }
        return std::make_unique<FieldDecl>(parseFieldDecl(std::move(modifiers), classContext, std::move(type), name));
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

    FieldDecl Parser::parseFieldDecl(std::vector<ModifierFlag> modifiers, jcutils::ClassDeclContext context,
        TypeNode type, const Token& firstName)
    {
        FieldDecl fieldDecl;
        if (const auto* invalid = jcutils::findInvalidFieldModifier(modifiers, context))
        {
            throw parser_exception(ParserExceptionType::ModifierNotAllowedForField, invalid->range);
        }
        if (const auto conflict = jcutils::findModifierConflict(modifiers, jcutils::ModifierDeclaration::Field))
        {
            throw parser_exception(conflict->exception, conflict->range);
        }
        fieldDecl.modifiers = std::move(modifiers);

        fieldDecl.type = std::move(type);

        const Token* name = &firstName;
        while (true)
        {
            VariableDeclarator variableDeclarator;
            variableDeclarator.name = name->content;
            variableDeclarator.dimensions = parseArrayDimension();
            if (checkSymbol(SymbolType::Assign))
            {
                //TODO: implement parse initializer
                throw parser_exception(ParserExceptionType::UnsupportedFieldInitializer, peek());
            }
            if (jcutils::isInterfaceBody(context))
            {
                throw parser_exception(ParserExceptionType::InterfaceFieldRequiresInitializer, peek());
            }
            fieldDecl.declarators.push_back(std::move(variableDeclarator));
            if (!matchSymbol(SymbolType::Comma))
            {
                break;
            }
            name = &expectIdentifier();
        }

        expectSymbol(SymbolType::Semicolon);

        return fieldDecl;
    }

    std::unique_ptr<InitializerDecl> Parser::parseInitializerDecl()
    {
        InitializerDecl decl;
        if (matchKeyword(STATIC))
        {
            decl.isStatic = true;
            decl.body = std::make_unique<BlockStatement>(parseMethodBody());
        } else
        {
            decl.body = std::make_unique<BlockStatement>(parseMethodBody());
        }
        return std::make_unique<InitializerDecl>(std::move(decl));
    }

    MethodDecl Parser::parseMethodDecl(std::vector<ModifierFlag> modifiers,
        jcutils::MethodDeclContext context, std::vector<GenericParameter> typeParameters,
        TypeNode returnType, const Token& returnTypeStart, const Token& name,
        std::optional<ReceiverParameter> receiverParameter)
    {
        MethodDecl methodDecl;
        if (const auto* invalid = jcutils::findInvalidMethodModifier(modifiers, context, sourceLangLevel))
        {
            throw parser_exception(ParserExceptionType::ModifierNotAllowedForMethod, invalid->range);
        }
        if (const auto conflict = jcutils::findModifierConflict(modifiers, jcutils::ModifierDeclaration::Method))
        {
            throw parser_exception(conflict->exception, conflict->range);
        }
        const auto abstractModifier = std::ranges::find(modifiers, FlagType::Abstract, &ModifierFlag::flagType);
        const bool isAbstract = abstractModifier != modifiers.end();
        if (isAbstract && context == jcutils::MethodDeclContext::Normal)
        {
            throw parser_exception(ParserExceptionType::AbstractMethodInNonAbstractClass, abstractModifier->range);
        }

        const bool annotationElement = context == jcutils::MethodDeclContext::WithinAnnotation;
        const bool isStatic = jcutils::haveModifier(modifiers, FlagType::Static);
        if (receiverParameter && (annotationElement || isStatic))
        {
            //TODO: Attach the receiver parameter token site.
            throw parser_exception(ParserExceptionType::ReceiverParameterNotAllowed);
        }

        const bool classMethod = context == jcutils::MethodDeclContext::Normal
            || context == jcutils::MethodDeclContext::WithinAbstractClass;
        const bool needsBody = classMethod
            ? !isAbstract && !jcutils::haveModifier(modifiers, FlagType::Native)
            : context == jcutils::MethodDeclContext::WithinInterface
                && (isStatic || jcutils::haveModifier(modifiers, FlagType::Default)
                    || jcutils::haveModifier(modifiers, FlagType::Private));
        methodDecl.modifiers = std::move(modifiers);
        if (annotationElement && !typeParameters.empty())
        {
            throw parser_exception(ParserExceptionType::TypeParametersNotAllowedForAnnotationElement, returnTypeStart);
        }
        methodDecl.typeParameters = std::move(typeParameters);
        methodDecl.returnType = std::move(returnType);
        methodDecl.receiverParam = std::move(receiverParameter);
        methodDecl.name = name.content;

        expectSymbol(SymbolType::LParen);
        if (annotationElement)
        {
            expectSymbol(SymbolType::RParen);
        }
        else
        {
            methodDecl.parameters = parseMethodParameters();
        }

        if (methodDecl.returnType.objectType != ObjectType::Void)
        {
            methodDecl.returnDimensions = parseArrayDimension();
        }

        if (annotationElement)
        {
            if (methodDecl.returnType.objectType == ObjectType::Void
                || methodDecl.returnType.dimensions.size() + methodDecl.returnDimensions.size() > 1)
            {
                throw parser_exception(ParserExceptionType::InvalidAnnotationElementType, returnTypeStart);
            }
            //TODO: Resolve named annotation element return types when type resolution is available.
            if (matchKeyword(DEFAULT))
            {
                methodDecl.defaultValue = parseElementValue();
            }
            expectSymbol(SymbolType::Semicolon);
            return methodDecl;
        }

        if (matchKeyword(THROWS))
        {
            do
            {
                const auto& typeStart = peek();
                TypeNode type = parseTypeNode();
                if (!jcutils::canExtImplTo(type))
                {
                    throw parser_exception(ParserExceptionType::InvalidThrowsType, typeStart);
                }
                methodDecl.thrownTypes.push_back(std::move(type));
            } while (matchSymbol(SymbolType::Comma));
        }

        if (needsBody)
        {
            methodDecl.body = std::make_unique<BlockStatement>(parseMethodBody());
        }
        else
        {
            expectSymbol(SymbolType::Semicolon);
        }

        return methodDecl;
    }

    ConstructorDecl Parser::parseConstructorDecl(std::vector<ModifierFlag> modifiers,
        jcutils::ClassDeclContext context, std::vector<GenericParameter> typeParameters, const Token& name,
        std::optional<ReceiverParameter> receiverParameter)
    {
        if (jcutils::isInterfaceBody(context))
        {
            throw parser_exception(ParserExceptionType::ConstructorNotAllowedInContext, name);
        }

        ConstructorDecl methodDecl;
        if (const auto* invalid = jcutils::findInvalidConstructorModifier(modifiers))
        {
            throw parser_exception(ParserExceptionType::ModifierNotAllowedForConstructor, invalid->range);
        }
        if (const auto conflict = jcutils::findModifierConflict(modifiers, jcutils::ModifierDeclaration::Constructor))
        {
            throw parser_exception(conflict->exception, conflict->range);
        }
        methodDecl.modifiers = std::move(modifiers);

        methodDecl.typeParameters = std::move(typeParameters);
        methodDecl.receiverParam = std::move(receiverParameter);
        methodDecl.name = name.content;

        expectSymbol(SymbolType::LParen);
        methodDecl.parameters = parseMethodParameters();

        if (matchKeyword(THROWS))
        {
            do
            {
                const auto& typeStart = peek();
                TypeNode type = parseTypeNode();
                if (!jcutils::canExtImplTo(type))
                {
                    throw parser_exception(ParserExceptionType::InvalidThrowsType, typeStart);
                }
                methodDecl.thrownTypes.push_back(std::move(type));
            } while (matchSymbol(SymbolType::Comma));
        }

        methodDecl.body = std::make_unique<BlockStatement>(parseMethodBody());

        return methodDecl;
    }

    std::vector<std::unique_ptr<ParameterDecl>> Parser::parseMethodParameters()
    {
        std::vector<std::unique_ptr<ParameterDecl>> decls;
        if (matchSymbol(SymbolType::RParen))
        {
            return decls;
        }

        do
        {
            auto parameter = std::make_unique<ParameterDecl>();
            bool seenFinal = false;
            while (checkKeyword(FINAL) || canBeAnnotation())
            {
                for (auto& annotation : parseAnnotations())
                {
                    ModifierFlag modifier;
                    modifier.range = annotation.range;
                    modifier.annotation = std::move(annotation);
                    parameter->modifiers.push_back(std::move(modifier));
                }

                if (checkKeyword(FINAL))
                {
                    if (seenFinal)
                    {
                        throw parser_exception(ParserExceptionType::DuplicateFinalInParameterTypeSpecification,peek());
                    }
                    seenFinal = true;
                    ModifierFlag modifier;
                    modifier.range = advance().range;
                    modifier.flagType = FlagType::Final;
                    parameter->modifiers.push_back(std::move(modifier));
                }
            }

            const Token& typeStart = peek();
            parameter->type = parseTypeNode({}, &parameter->varargs);
            if (parameter->type->objectType == ObjectType::Void)
            {
                throw parser_exception(ParserExceptionType::VoidAsTypeSpecification,typeStart);
            }
            parameter->name = expectIdentifier().content;
            if (!parameter->varargs)
            {
                parameter->dimensions = parseArrayDimension();
            }

            if (extensionEnabled(JavacExtension::AllowDefaultParameters) && checkSymbol(SymbolType::Assign))
            {
                //TODO: Implement default value parsing (extension)
                throw parser_exception(ParserExceptionType::UnsupportedParameterDefault, peek());
            }

            const bool isVarargs = parameter->varargs.has_value();
            decls.push_back(std::move(parameter));
            if (isVarargs)
            {
                break;
            }
        } while (matchSymbol(SymbolType::Comma));

        expectSymbol(SymbolType::RParen);
        return decls;
    }

    BlockStatement Parser::parseMethodBody()
    {
        expectSymbol(SymbolType::LBrace);

        //TODO: Implement parse the body

        expectSymbol(SymbolType::RBrace);
        return {};
    }

    std::vector<Annotation> Parser::parseAnnotations()
    {
        std::vector<Annotation> annotations;
        while (canBeAnnotation())
        {
            annotations.push_back(parseAnnotation());
        }
        return annotations;
    }

    Annotation Parser::parseAnnotation()
    {
        Annotation annotation;
        annotation.range = expectSymbol(SymbolType::AtSign).range;

        const auto& firstName = expectIdentifier();
        annotation.name = firstName.content;
        annotation.range->end = firstName.range.end;
        while (matchSymbol(SymbolType::Dot))
        {
            const auto& part = expectIdentifier();
            annotation.name += u'.';
            annotation.name += part.content;
            annotation.range->end = part.range.end;
        }

        if (matchSymbol(SymbolType::LParen))
        {
            if (checkIdentifier() && peek(1).symbol == SymbolType::Assign)
            {
                do
                {
                    annotation.arguments.push_back(parseAnnotationArg());
                } while (matchSymbol(SymbolType::Comma));
            }
            else if (!checkSymbol(SymbolType::RParen))
            {
                AnnotationArg arg;
                arg.value = parseElementValue();
                annotation.arguments.push_back(std::move(arg));
            }

            annotation.range->end = expectSymbol(SymbolType::RParen).range.end;
        }

        return annotation;
    }

    AnnotationArg Parser::parseAnnotationArg()
    {
        AnnotationArg arg = {};
        arg.name = expectIdentifier().content;
        expectSymbol(SymbolType::Assign);
        arg.value = parseElementValue();
        return arg;
    }

    std::unique_ptr<Expr> Parser::parseElementValue()
    {
        if (checkSymbol(SymbolType::AtSign))
        {
            return std::make_unique<Annotation>(parseAnnotation());
        }

        if (matchSymbol(SymbolType::LBrace))
        {
            return parseElementArray();
        }

        return parseElementExpr();
    }

    std::unique_ptr<Expr> Parser::parseElementArray()
    {
        ArrayInitializerExpr arrayInitializer = {};
        if (!checkSymbol(SymbolType::RBrace) && !matchSymbol(SymbolType::Comma))
        {
            do
            {
                arrayInitializer.elements.push_back(parseElementValue());
            } while (matchSymbol(SymbolType::Comma) && !checkSymbol(SymbolType::RBrace));
        }
        expectSymbol(SymbolType::RBrace);
        return std::make_unique<ArrayInitializerExpr>(std::move(arrayInitializer));
    }

    std::unique_ptr<Expr> Parser::parseElementExpr()
    {
        //TODO: implement
        throw parser_exception(ParserExceptionType::UnsupportedElementExpression, peek());
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
                stringExpr.value = token.content;
                return  std::make_unique<StringExpr>(stringExpr);
            }
            case TokenType::CharacterLiteral:
            {
                CharExpr charExpr;
                charExpr.value = token.content.front();
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
