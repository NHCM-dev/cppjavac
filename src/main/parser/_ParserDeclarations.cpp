#include "Parser.h"

#include <algorithm>
#include <utility>

#include "ParserException.h"
#include "ParserUtil.h"

namespace javac
{
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

    ImportDecl Parser::parseImportDecl()
    {
        expectKeyword(IMPORT);

        ImportDecl importDecl;

        std::u16string importString;

        if (matchKeyword(STATIC))
        {
            importDecl.importType = ImportType::Static;
            importString += expectIdentifier().content;
            while (matchSymbol(SymbolType::Dot))
            {
                importString += u'.';
                if (matchSymbol(SymbolType::Star))
                {
                    importDecl.importType = ImportType::StaticOnDemand;
                    break;
                }
                importString += expectIdentifier().content;
            }
            expectSymbol(SymbolType::Semicolon);
            return importDecl;
        }
        if (langLevelMeet(LangLevel::TwentyFive) && matchKeyword(MODULE))
        {
            importDecl.importType = ImportType::Module;
            importString += expectIdentifier().content;
            while (matchSymbol(SymbolType::Dot))
            {
                importString += u'.';
                importString += expectIdentifier().content;
            }
            expectSymbol(SymbolType::Semicolon);
            return importDecl;
        }
        importDecl.importType = ImportType::Type;
        importString += expectIdentifier().content;
        while (matchSymbol(SymbolType::Dot))
        {
            importString += u'.';
            if (matchSymbol(SymbolType::Star))
            {
                importDecl.importType = ImportType::TypeOnDemand;
                break;
            }
            importString += expectIdentifier().content;
        }
        expectSymbol(SymbolType::Semicolon);
        return importDecl;
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

        while (Uptr<Decl> member = parseNormalClassMember(classDecl.name, bodyContext, methodContext))
        {
            classDecl.members.push_back(std::move(member));
        }

        expectSymbol(SymbolType::RBrace);

        return classDecl;
    }

    Uptr<Decl> Parser::parseNormalClassMember(std::u16string_view className,
        jcutils::ClassDeclContext classContext, jcutils::MethodDeclContext methodContext,
        std::optional<ReceiverParameter> receiverParameter)
    {
        while (matchSymbol(SymbolType::Semicolon)){}

        // Nothing passed
        if (checkSymbol(SymbolType::RBrace))
        {
            return nullptr;
        }
        
        const bool isAnonymous = classContext == jcutils::ClassDeclContext::Anonymous;
        const bool restrictAnonymousStatics = isAnonymous && !langLevelMeet(LangLevel::Sixteen);

        // Try parse initializer such as static block
        bool isStatic = checkKeyword(STATIC) && peek(1).symbol == SymbolType::LBrace;
        if (isStatic)
        {
            if (restrictAnonymousStatics)
            {
                throw parser_exception(ParserExceptionType::StaticMemberInAnonymousClassBeforeJava16, peek());
            }
            if (!isAnonymous && classContext != jcutils::ClassDeclContext::TopLevel)
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
        const bool restrictedStatic = restrictAnonymousStatics && jcutils::haveModifier(modifiers, FlagType::Static);

        // Check for class
        if (checkKeyword(CLASS))
        {
            if (restrictedStatic)
            {
                throw parser_exception(ParserExceptionType::StaticMemberInAnonymousClassBeforeJava16, peek());
            }
            return makeUniq<ClassDecl>(parseClassDecl(std::move(modifiers), jcutils::ClassDeclContext::Member));
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
            if (isAnonymous)
            {
                throw parser_exception(ParserExceptionType::ConstructorInAnonymousClass, name);
            }
            if (name.content != className)
            {
                throw parser_exception(ParserExceptionType::ConstructorNameMismatch, name);
            }
            return makeUniq<ConstructorDecl>(parseConstructorDecl(std::move(modifiers), classContext,
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
            if (restrictedStatic)
            {
                throw parser_exception(ParserExceptionType::StaticMemberInAnonymousClassBeforeJava16, name);
            }
            return makeUniq<MethodDecl>(
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
        return makeUniq<FieldDecl>(parseFieldDecl(std::move(modifiers), classContext, std::move(type), name));
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

        const bool requiresConstantInitializer = context == jcutils::ClassDeclContext::Anonymous
            && !langLevelMeet(LangLevel::Sixteen) && jcutils::haveModifier(fieldDecl.modifiers, FlagType::Static);
        if (requiresConstantInitializer && !jcutils::haveModifier(fieldDecl.modifiers, FlagType::Final))
        {
            throw parser_exception(ParserExceptionType::StaticMemberInAnonymousClassBeforeJava16, firstName);
        }

        const Token* name = &firstName;
        while (true)
        {
            VariableDeclarator variableDeclarator;
            variableDeclarator.name = name->content;
            variableDeclarator.dimensions = parseArrayDimension();
            if (requiresConstantInitializer && (fieldDecl.type.objectType == ObjectType::Array
                || !variableDeclarator.dimensions.empty() || !checkSymbol(SymbolType::Assign)))
            {
                throw parser_exception(ParserExceptionType::StaticMemberInAnonymousClassBeforeJava16, *name);
            }
            if (matchSymbol(SymbolType::Assign))
            {
                variableDeclarator.initializer = parseVariableInitializer();
            }
            else if (jcutils::isInterfaceBody(context))
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

    VariableDecl Parser::parseVariableDecl(std::vector<ModifierFlag> modifiers, bool consumeSemiColon,
        jcutils::VariableDeclContext context)
    {
        VariableDecl declaration;
        declaration.range = peek().range;
        if (modifiers.empty())
        {
            modifiers = parseModifierFlags();
        }
        if (!modifiers.empty() && modifiers.front().range)
        {
            declaration.range->begin = modifiers.front().range->begin;
        }

        bool seenFinal = false;
        for (const auto& modifier : modifiers)
        {
            if (!modifier.flagType)
            {
                continue;
            }
            if (modifier.flagType != FlagType::Final)
            {
                throw parser_exception(ParserExceptionType::ModifierNotAllowedForLocalVariable, modifier.range);
            }
            if (seenFinal)
            {
                throw parser_exception(ParserExceptionType::DuplicateModifier, modifier.range);
            }
            seenFinal = true;
        }
        declaration.modifiers = std::move(modifiers);

        const Token typeStart = peek();
        const bool inferred = checkKeyword(VAR) && peek(1).symbol != SymbolType::Dot;
        const bool enhancedFor = context == jcutils::VariableDeclContext::EnhancedFor;
        const bool tryResource = context == jcutils::VariableDeclContext::TryResource;
        if (inferred)
        {
            declaration.type.objectType = ObjectType::Inferred;
            declaration.type.range = advance().range;
        }
        else
        {
            declaration.type = parseTypeNode();
            if (declaration.type.objectType == ObjectType::Void)
            {
                throw parser_exception(ParserExceptionType::VoidAsTypeSpecification, typeStart);
            }
        }

        do
        {
            VariableDeclarator declarator;
            const Token& name = expectIdentifier();
            declarator.range = name.range;
            declarator.name = name.content;
            const Token dimensionsStart = peek();
            declarator.dimensions = parseArrayDimension();
            if (inferred && !declarator.dimensions.empty())
            {
                throw parser_exception(ParserExceptionType::ArrayDimensionInInferredVar,dimensionsStart);
            }

            if (tryResource && !declarator.dimensions.empty())
            {
                throw parser_exception(ParserExceptionType::ArrayDimensionInTryResource,dimensionsStart);
            }

            if (checkSymbol(SymbolType::Assign))
            {
                if (enhancedFor)
                {
                    throw parser_exception(ParserExceptionType::InitializerNotAllowedForEnhancedFor, peek());
                }
                advance();
                if (inferred && checkSymbol(SymbolType::LBrace))
                {
                    throw parser_exception(ParserExceptionType::ArrayInitializerInInferredVar,peek());
                }
                declarator.initializer = parseVariableInitializer();
            }
            else if (tryResource)
            {
                throw parser_exception(ParserExceptionType::ExpectAssign, peek());
            }
            else if (inferred && !enhancedFor)
            {
                throw parser_exception(ParserExceptionType::ExpectInitializerForInferredVar,name);
            }

            declarator.range->end = tokens().at(tokenPointer - 1).range.end;
            declaration.declarators.push_back(std::move(declarator));
            if ((inferred || enhancedFor || tryResource) && checkSymbol(SymbolType::Comma))
            {
                throw parser_exception(ParserExceptionType::MultipleVarDeclarations, peek());
            }
        } while (matchSymbol(SymbolType::Comma));

        declaration.range->end = declaration.declarators.back().range->end;
        if (consumeSemiColon)
        {
            declaration.range->end = expectSymbol(SymbolType::Semicolon).range.end;
        }
        return declaration;
    }

    Uptr<InitializerDecl> Parser::parseInitializerDecl()
    {
        InitializerDecl decl;
        if (matchKeyword(STATIC))
        {
            decl.isStatic = true;
            decl.body = parseBlock();
        } else
        {
            decl.body = parseBlock();
        }
        return makeUniq<InitializerDecl>(std::move(decl));
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
            methodDecl.body = parseBlock();
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

        methodDecl.body = parseBlock();

        return methodDecl;
    }

    std::vector<Uptr<ParameterDecl>> Parser::parseMethodParameters()
    {
        std::vector<Uptr<ParameterDecl>> decls;
        if (matchSymbol(SymbolType::RParen))
        {
            return decls;
        }

        do
        {
            auto parameter = makeUniq<ParameterDecl>();
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
} // javac
