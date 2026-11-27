#include "Parser.h"

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

    Uptr<Expr> Parser::parseConstantValue()
    {
        //TODO: implement
        throw parser_exception(ParserExceptionType::UnsupportedExpression, peek());
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

    Uptr<Expr> Parser::parseValue()
    {
        //TODO: implement
        throw parser_exception(ParserExceptionType::UnsupportedExpression, peek());
    }
} // javac
