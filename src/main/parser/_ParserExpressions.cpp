#include "Parser.h"

#include <algorithm>
#include <utility>

#include "ParserException.h"

namespace javac
{
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

    Uptr<Expr> Parser::parseElementValue()
    {
        if (checkSymbol(SymbolType::AtSign))
        {
            return makeUniq<Annotation>(parseAnnotation());
        }

        if (matchSymbol(SymbolType::LBrace))
        {
            return parseElementArray();
        }

        return parseElementExpr();
    }

    Uptr<Expr> Parser::parseElementArray()
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
        return makeUniq<ArrayInitializerExpr>(std::move(arrayInitializer));
    }

    Uptr<Expr> Parser::parseElementExpr()
    {
        //TODO: implement
        throw parser_exception(ParserExceptionType::UnsupportedElementExpression, peek());
    }

    Uptr<Expr> Parser::parseBasicLiteral()
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
                    return  makeUniq<LongExpr>(longExpr);
                }
                IntExpr intExpr;
                intExpr.value = static_cast<int32_t>(number);
                return  makeUniq<IntExpr>(intExpr);
            }
            case TokenType::FloatLiteral:
            {
                try
                {
                    if (jcutils::is32Float(token.content))
                    {
                        FloatExpr floatExpr;
                        floatExpr.value = jcutils::floatFromString(token.content);
                        return makeUniq<FloatExpr>(floatExpr);
                    }

                    DoubleExpr doubleExpr;
                    doubleExpr.value = jcutils::doubleFromString(token.content);
                    return makeUniq<DoubleExpr>(doubleExpr);
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
                return  makeUniq<StringExpr>(stringExpr);
            }
            case TokenType::CharacterLiteral:
            {
                CharExpr charExpr;
                charExpr.value = token.content.front();
                return  makeUniq<CharExpr>(charExpr);
            }
            case TokenType::BooleanLiteral:
            {
                BooleanExpr boolExpr;
                boolExpr.value = jcutils::boolFromString(token.content);
                return makeUniq<BooleanExpr>(boolExpr);
            }
            case TokenType::NullLiteral:
            {
                NullExpr nullExpr;
                return makeUniq<NullExpr>(nullExpr);
            }
            default:
            {
                throw parser_exception(ParserExceptionType::NotLiteral, token);
            }
        }
    }

    Uptr<ClassLiteralExpr> Parser::parseClassLiteral()
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
                    array.elementType = makeUniq<TypeNode>(std::move(node));
                    array.dimensions = std::move(dimensions);
                    node = std::move(array);
                }
            }

            expectSymbol(SymbolType::Dot);
            expectKeyword(CLASS);
            auto literal = makeUniq<ClassLiteralExpr>();
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

    Uptr<Expr> Parser::parseStatementExpression()
    {
        const Token start = peek();
        Uptr<Expr> expression = parseValue();
        if (!dynamic_cast<const AssignExpr*>(expression.get())
            && !dynamic_cast<const IncDecExpr*>(expression.get())
            && !dynamic_cast<const CallExpr*>(expression.get())
            && !dynamic_cast<const NewExpr*>(expression.get()))
        {
            throw parser_exception(ParserExceptionType::ExpectStatementExpression, start);
        }
        return expression;
    }

    Uptr<Expr> Parser::parseVariableInitializer()
    {
        if (!checkSymbol(SymbolType::LBrace))
        {
            return parseValue();
        }

        auto array = makeUniq<ArrayInitializerExpr>();
        array->range = expectSymbol(SymbolType::LBrace).range;
        if (!checkSymbol(SymbolType::RBrace) && !matchSymbol(SymbolType::Comma))
        {
            do
            {
                array->elements.push_back(parseVariableInitializer());
            } while (matchSymbol(SymbolType::Comma) && !checkSymbol(SymbolType::RBrace));
        }
        array->range->end = expectSymbol(SymbolType::RBrace).range.end;
        return array;
    }

    Uptr<Expr> Parser::parseConstantValue()
    {
        //TODO: implement
        throw parser_exception(ParserExceptionType::UnsupportedExpression, peek());
    }

    Uptr<Expr> Parser::parseValue()
    {
        //TODO: implement

        if (checkSymbol(SymbolType::LParen))
        {
            return parseParenExpr();
        }

    }

    Uptr<BinaryExpr> Parser::parseBinaryExpr()
    {

    }


    Uptr<ArithmeticExpr> Parser::parseAdd()
    {

    }

    Uptr<ArithmeticExpr> Parser::parseMinus()
    {

    }

    Uptr<ArithmeticExpr> Parser::parseMultiply()
    {

    }

    Uptr<ArithmeticExpr> Parser::parseSubtract()
    {

    }

    Uptr<ArithmeticExpr> Parser::parseModulo()
    {

    }


    Uptr<UnaryExpr> Parser::parseUnaryExpr()
    {

    }

    Uptr<NameExpr> Parser::parseNameExpr()
    {
        Uptr<NameExpr> expr = makeUniq<NameExpr>();
        expr->name = expectIdentifier().content;
        return expr;
    }

    Uptr<ParenExpr> Parser::parseParenExpr()
    {
        Uptr<ParenExpr> expr = makeUniq<ParenExpr>();

        expectSymbol(SymbolType::LParen);
        if (checkSymbol(SymbolType::RParen))
        {
            throw parser_exception(ParserExceptionType::UnsupportedExpression, peek());
        }
        expr->expression = parseValue();
        expectSymbol(SymbolType::RParen);

        return expr;
    }

    Uptr<ThisExpr> Parser::parseThisExpr(std::u16string qualifier)
    {
        expectKeyword(THIS);
        Uptr<ThisExpr> expr = makeUniq<ThisExpr>();
        expr->qualifier = std::move(qualifier);
        return expr;
    }

    Uptr<SuperExpr> Parser::parseSuperExpr(std::u16string qualifier)
    {
        expectKeyword(SUPER);
        Uptr<SuperExpr> expr = makeUniq<SuperExpr>();
        expr->qualifier = std::move(qualifier);
        return expr;
    }

    Uptr<FieldAccessExpr> Parser::parseFieldAccessExpr(Uptr<Expr> receiver)
    {
        Uptr<FieldAccessExpr> expr = makeUniq<FieldAccessExpr>();

        expr->receiver = std::move(receiver);

        expectSymbol(SymbolType::Dot);

        expr->name = expectIdentifier().content;

        return expr;
    }

    Uptr<ArrayAccessExpr> Parser::parseArrayAccessExpr(Uptr<Expr> arrayExpr)
    {
        Uptr<ArrayAccessExpr> expr = makeUniq<ArrayAccessExpr>();

        expr->arrayExpr = std::move(arrayExpr);

        expectSymbol(SymbolType::LBracket);

        expr->indexExpr = parseValue();

        expectSymbol(SymbolType::RBracket);

        return expr;
    }

    Uptr<CallExpr> Parser::parseCallExpr(Uptr<Expr> callee, std::vector<TypeNode> typeArguments)
    {
        Uptr<CallExpr> expr = makeUniq<CallExpr>();

        expr->callee = std::move(callee);
        expr->typeArguments = std::move(typeArguments);

        expectSymbol(SymbolType::LParen);
        if (matchSymbol(SymbolType::RParen))
        {
            return expr;
        }

        do
        {
            expr->arguments.push_back(parseValue());
        } while (matchSymbol(SymbolType::Comma));

        expectSymbol(SymbolType::RParen);

        return expr;
    }

    Uptr<NewExpr> Parser::parseNewExpr(Uptr<Expr> qualifier)
    {
        expectKeyword(NEW);

        Uptr<NewExpr> expr = makeUniq<NewExpr>();

        expr->qualifier = std::move(qualifier);
        if (matchSymbol(SymbolType::LAngleBracket))
        {
            do
            {
                const Token argumentStart = peek();
                auto argument = parseTypeNode();
                if (!jcutils::canGeneric(argument))
                {
                    throw parser_exception(ParserExceptionType::InvalidExactGeneric, argumentStart);
                }
                expr->typeArguments.push_back(std::move(argument));
            } while (matchSymbol(SymbolType::Comma));
            expectTypeClose();
        }

        const Token typeStart = peek();
        expr->type = parseTypeNode({}, nullptr, expr->typeArguments.empty());
        if (!jcutils::isActualClass(expr->type))
        {
            throw parser_exception(ParserExceptionType::InvalidClassInstanceCreationType, typeStart);
        }
        for (const auto& part : expr->type.nameParts)
        {
            if (&part != &expr->type.nameParts.back() && !part.typeArguments.empty())
            {
                throw parser_exception(ParserExceptionType::InvalidClassInstanceCreationType, typeStart);
            }
            for (const auto& argument : part.typeArguments)
            {
                if (argument.genericType != GenericType::Exact)
                {
                    throw parser_exception(ParserExceptionType::InvalidClassInstanceCreationType, typeStart);
                }
            }
        }

        expectSymbol(SymbolType::LParen);
        if (!checkSymbol(SymbolType::RParen))
        {
            do
            {
                expr->arguments.push_back(parseValue());
            } while (matchSymbol(SymbolType::Comma));
        }
        expectSymbol(SymbolType::RParen);

        if (matchSymbol(SymbolType::LBrace))
        {
            auto& members = expr->members.emplace();

            while (Uptr<Decl> member = parseNormalClassMember({},
                jcutils::ClassDeclContext::Anonymous,
                jcutils::MethodDeclContext::Normal))
            {
                members.push_back(std::move(member));
            }

            expectSymbol(SymbolType::RBrace);
        }
        return expr;
    }

    Uptr<ArrayInitializerExpr> Parser::parseArrayInitializerExpr()
    {
        Uptr<ArrayInitializerExpr> expr = makeUniq<ArrayInitializerExpr>();

        expectSymbol(SymbolType::LBrace);
        if (!checkSymbol(SymbolType::RBrace) && !matchSymbol(SymbolType::Comma))
        {
            do
            {
                if (checkSymbol(SymbolType::LBrace))
                {
                    expr->elements.push_back(parseArrayInitializerExpr());
                }
                else
                {
                    expr->elements.push_back(parseValue());
                }
            } while (matchSymbol(SymbolType::Comma) && !checkSymbol(SymbolType::RBrace));
        }
        expectSymbol(SymbolType::RBrace);

        return expr;
    }

    Uptr<ArrayDimensionExpr> Parser::parseArrayDimensionExpr(std::vector<Annotation> annotations)
    {
        Uptr<ArrayDimensionExpr> expr = makeUniq<ArrayDimensionExpr>();

        expr->annotations = std::move(annotations);

        expectSymbol(SymbolType::LBracket);
        if (matchSymbol(SymbolType::RBracket))
        {
            return expr;
        }
        expr->sizeExpr = parseValue();
        expectSymbol(SymbolType::RBracket);

        return expr;
    }

    Uptr<NewArrayExpr> Parser::parseNewArrayExpr()
    {
        Uptr<NewArrayExpr> expr = makeUniq<NewArrayExpr>();

        expectKeyword(NEW);

        const Token typeStart = peek();
        expr->elementType = parseTypeNode({}, nullptr, false, false);
        if (expr->elementType.objectType != ObjectType::Primitive && !jcutils::isActualClass(expr->elementType))
        {
            throw parser_exception(ParserExceptionType::InvalidArrayCreationType, typeStart);
        }

        bool hasSizedDimension = false;
        bool hasUnsizedDimension = false;
        do
        {
            std::vector<Annotation> annotations = parseAnnotations();
            const Token dimensionStart = peek();
            if (hasUnsizedDimension && checkSymbol(SymbolType::LBracket)
                && peek(1).symbol != SymbolType::RBracket)
            {
                throw parser_exception(ParserExceptionType::SizedDimensionAfterUnsizedDimension, dimensionStart);
            }

            Uptr<ArrayDimensionExpr> dimension = parseArrayDimensionExpr(std::move(annotations));
            hasSizedDimension |= static_cast<bool>(dimension->sizeExpr);
            hasUnsizedDimension |= !dimension->sizeExpr;
            expr->dimensions.push_back(std::move(*dimension));
        } while (checkSymbol(SymbolType::LBracket) || canBeAnnotation());

        if (hasSizedDimension)
        {
            if (checkSymbol(SymbolType::LBrace))
            {
                throw parser_exception(ParserExceptionType::ArrayInitializerWithSizedDimension, peek());
            }
        }
        else
        {
            if (!checkSymbol(SymbolType::LBrace))
            {
                throw parser_exception(ParserExceptionType::ExpectArrayInitializer, peek());
            }
            expr->initializer = parseArrayInitializerExpr();
        }

        return expr;
    }

    Uptr<CastExpr> Parser::parseCastExpr()
    {
        Uptr<CastExpr> expr = makeUniq<CastExpr>();

        expectSymbol(SymbolType::LParen);
        const Token& start = peek();
        if (matchSymbol(SymbolType::RParen))
        {
            throw parser_exception(ParserExceptionType::ExpectTypeSpecification, start);
        }

        expr->castType = parseTypeNode(parseAnnotations());

        while (matchSymbol(SymbolType::Comma))
        {
            expr->additionalBounds.push_back(parseTypeNode(parseAnnotations()));
        }
        expectSymbol(SymbolType::RParen);

        expr->value = parseValue();

        return expr;
    }

    Uptr<InstanceOfExpr> Parser::parseInstanceOfExpr(Uptr<Expr> value)
    {
        expectKeyword(INSTANCEOF);
        Uptr<InstanceOfExpr> expr = makeUniq<InstanceOfExpr>();
        expr->value = std::move(value);
        expr->pattern = parseInstanceOfPattern(&expr->testType);
        return expr;
    }

    Uptr<Pattern> Parser::parseInstanceOfPattern(std::optional<TypeNode>* testType)
    {
        const Token start = peek();
        const bool isComponent = testType == nullptr;
        const bool allowUnnamedPatterns = langLevelMeet(LangLevel::TwentyFive);
        if (isComponent && allowUnnamedPatterns && matchKeyword(UNDERSCORE))
        {
            return makeUniq<UnnamedPattern>();
        }

        std::vector<ModifierFlag> modifiers = parseModifierFlags();
        bool hasFinal = false;
        for (const auto& modifier : modifiers)
        {
            if (modifier.flagType)
            {
                if (hasFinal || modifier.flagType != FlagType::Final)
                {
                    throw parser_exception(ParserExceptionType::InvalidPattern, modifier.range);
                }
                hasFinal = true;
            }
        }

        TypeNode type;
        if (matchKeyword(VAR))
        {
            if (!isComponent)
            {
                throw parser_exception(ParserExceptionType::InvalidInstanceOfType, start);
            }
            type.objectType = ObjectType::Inferred;
        }
        else
        {
            type = parseTypeNode();
        }
        if (type.objectType == ObjectType::Void || (!isComponent && !jcutils::canGeneric(type)))
        {
            throw parser_exception(ParserExceptionType::InvalidInstanceOfType, start);
        }

        if (langLevelMeet(LangLevel::TwentyOne) && checkSymbol(SymbolType::LParen))
        {
            if (!modifiers.empty() || !jcutils::isActualClass(type))
            {
                throw parser_exception(ParserExceptionType::InvalidPattern, start);
            }
            for (const auto& part : type.nameParts)
            {
                if (!part.annotations.empty())
                {
                    throw parser_exception(ParserExceptionType::InvalidPattern, start);
                }
            }

            advance();
            Uptr<RecordPattern> pattern = makeUniq<RecordPattern>();
            pattern->type = std::move(type);
            if (!checkSymbol(SymbolType::RParen))
            {
                do
                {
                    pattern->components.push_back(parseInstanceOfPattern());
                } while (matchSymbol(SymbolType::Comma));
            }
            expectSymbol(SymbolType::RParen);
            return pattern;
        }

        const bool unnamedBinding = allowUnnamedPatterns && checkKeyword(UNDERSCORE);
        if (langLevelMeet(LangLevel::Sixteen) && (unnamedBinding || checkIdentifier() || isComponent))
        {
            auto pattern = makeUniq<TypePattern>();
            pattern->modifiers = std::move(modifiers);
            pattern->type = std::move(type);
            pattern->name = unnamedBinding ? advance().content : expectIdentifier().content;
            if (checkSymbol(SymbolType::LBracket) || canBeAnnotation() || checkSymbol(SymbolType::Assign))
            {
                throw parser_exception(ParserExceptionType::InvalidPattern, peek());
            }
            return pattern;
        }

        if (hasFinal)
        {
            throw parser_exception(ParserExceptionType::InvalidPattern, start);
        }
        TypeNode& baseType = type.objectType == ObjectType::Array ? *type.elementType : type;
        std::vector<Annotation>& annotations = jcutils::isActualClass(baseType) ? baseType.nameParts.front().annotations : baseType.annotations;
        for (auto& modifier : modifiers)
        {
            annotations.push_back(std::move(*modifier.annotation));
        }
        testType->emplace(std::move(type));
        return nullptr;
    }

    Uptr<LambdaExpr> Parser::parseLambdaExpr()
    {
        Uptr<LambdaExpr> expr = makeUniq<LambdaExpr>();

        const auto readInferredParameter = [this, &expr]
        {
            Uptr<ParameterDecl> parameter = makeUniq<ParameterDecl>();
            parameter->name = expectIdentifier().content;
            expr->parameters.push_back(std::move(parameter));
        };

        if (!matchSymbol(SymbolType::LParen))
        {
            // x -> ...
            readInferredParameter();
        }
        else if (!matchSymbol(SymbolType::RParen))
        {
            if (checkIdentifier()
                && (peek(1).symbol == SymbolType::Comma
                    || peek(1).symbol == SymbolType::RParen))
            {
                // (x) -> ... or (x, y) -> ...
                do
                {
                    readInferredParameter();
                } while (matchSymbol(SymbolType::Comma));

                expectSymbol(SymbolType::RParen);
            }
            else
            {
                // (int x, String y) -> ...
                expr->parameters = parseMethodParameters();
            }
        }

        expectSymbol(SymbolType::Arrow);
        if (checkSymbol(SymbolType::LBrace))
        {
            expr->blockBody = parseBlock();
        }
        else
        {
            expr->exprBody = parseValue();
        }

        return expr;
    }

    Uptr<MethodRefExpr> Parser::parseMethodRefExpr(Uptr<Expr> exprQualifier, std::optional<TypeNode> typeQualifier)
    {
        const Token start = expectSymbol(SymbolType::DoubleColon);
        if (static_cast<bool>(exprQualifier) == typeQualifier.has_value()
            || (typeQualifier && !jcutils::canGeneric(*typeQualifier)))
        {
            throw parser_exception(ParserExceptionType::InvalidMethodReference, start);
        }

        Uptr<MethodRefExpr> expr = makeUniq<MethodRefExpr>();
        expr->exprQualifier = std::move(exprQualifier);
        expr->typeQualifier = std::move(typeQualifier);

        if (matchSymbol(SymbolType::LAngleBracket))
        {
            do
            {
                const Token argumentStart = peek();
                TypeNode argument = parseTypeNode();
                if (!jcutils::canGeneric(argument))
                {
                    throw parser_exception(ParserExceptionType::InvalidExactGeneric, argumentStart);
                }
                expr->typeArguments.push_back(std::move(argument));
            } while (matchSymbol(SymbolType::Comma));
            expectTypeClose();
        }

        if (checkKeyword(NEW))
        {
            if (!expr->typeQualifier)
            {
                TypeNode type;
                const Expr* qualifier = expr->exprQualifier.get();
                while (const FieldAccessExpr* field = dynamic_cast<const FieldAccessExpr*>(qualifier))
                {
                    TypeNamePart part;
                    part.name = field->name;
                    type.nameParts.push_back(std::move(part));
                    qualifier = field->receiver.get();
                }
                const NameExpr* name = dynamic_cast<const NameExpr*>(qualifier);
                if (!name)
                {
                    throw parser_exception(ParserExceptionType::InvalidMethodReference, peek());
                }
                TypeNamePart firstPart;
                firstPart.name = name->name;
                type.nameParts.push_back(std::move(firstPart));
                std::reverse(type.nameParts.begin(), type.nameParts.end());
                expr->typeQualifier.emplace(std::move(type));
                expr->exprQualifier.reset();
            }
            if (expr->typeQualifier->objectType == ObjectType::Array && !expr->typeArguments.empty())
            {
                throw parser_exception(ParserExceptionType::InvalidMethodReference, peek());
            }
            advance();
        }
        else
        {
            expr->methodName = expectIdentifier().content;
        }
        return expr;
    }

    Uptr<SwitchExpr> Parser::parseSwitchExpr()
    {
        Uptr<SwitchExpr> expr = makeUniq<SwitchExpr>();
        expr->range = expectKeyword(SWITCH).range;
        expectSymbol(SymbolType::LParen);
        expr->selectorExpr = parseValue();
        expectSymbol(SymbolType::RParen);
        expr->cases = parseSwitchCases(*expr->range);
        return expr;
    }

} // javac
