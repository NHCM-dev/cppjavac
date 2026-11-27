//
// Created by NHCM on 2026/11/25.
//

#ifndef CPPJAVAC_PARSEREXCEPTION_H
#define CPPJAVAC_PARSEREXCEPTION_H

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

#include "main/lexer/LexerDef.h"

namespace javac
{
    enum class ParserExceptionType : std::uint8_t
    {
        EXTENSION_VoidInGeneric,

        ExpectSymbol,
        ExpectKeyword,
        ExpectIdentifier,
        ExpectClassLiteral,

        MultipleExtends,
        MultipleImplements,
        MultiplePermits,

        NeedBraceForClassDeclaration,

        NotLiteral,
        
        OutOfRangeNumberLiteral,

        KeywordInTypeSpecification,
        KeywordInParameterTypeSpecification,
        KeywordInClassLiteral,

        VoidAsTypeSpecification,

        DuplicateFinalInParameterTypeSpecification,
        DuplicateModifier,

        TypeArgUnboundUndefinedBehaviour,

        AnnotationInArrayDimension,

        CannotExtendsTo,
        CannotBeSuper,
        CannotImplementsTo,
        CannotPermitsTo,


        GenericInClassLiteral,
        UnsupportedParameterDefault,

        ModifierNotAllowedForClass,
        ModifierNotAllowedForField,
        ModifierNotAllowedForMethod,
        ModifierNotAllowedForConstructor,

        ConflictingAccessModifiers,
        ConflictingClassModifiers,
        ConflictingFieldModifiers,
        ConflictingMethodModifiers,

        ConstructorNotAllowedInContext,

        ReceiverParameterNotAllowed,

        TypeParametersNotAllowedForAnnotationElement,

        InvalidNumberLiteral,
        InvalidExactGeneric,
        InvalidThrowsType,
        InvalidAnnotationElementType,

        InterfaceFieldRequiresInitializer
    };

    class parser_exception : public std::logic_error
    {
    public:
        const ParserExceptionType exception;
        const std::optional<Token> actualToken = std::nullopt;

        parser_exception(ParserExceptionType exception)
            : logic_error(""), exception(exception)
        {

        }

        parser_exception(ParserExceptionType exception, Token actualToken)
            : logic_error(""), exception(exception), actualToken(actualToken)
        {

        }

        parser_exception(ParserExceptionType exception, Token actualToken, const std::string& cause)
            : logic_error(cause), exception(exception), actualToken(actualToken)
        {

        }
    };
}

#endif //CPPJAVAC_PARSEREXCEPTION_H
