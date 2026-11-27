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
        ExpectColon,
        ExpectColonOrArrow,

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

        ModifierNotAllowedForClass,
        ModifierNotAllowedForField,
        ModifierNotAllowedForMethod,
        ModifierNotAllowedForConstructor,

        ConflictingAccessModifiers,
        ConflictingClassModifiers,
        ConflictingFieldModifiers,
        ConflictingMethodModifiers,
        AbstractMethodInNonAbstractClass,

        ConstructorNotAllowedInContext,

        ReceiverParameterNotAllowed,

        TypeParametersNotAllowedForAnnotationElement,
        TypeParametersNotAllowedForField,

        InvalidNumberLiteral,
        InvalidExactGeneric,
        InvalidThrowsType,
        InvalidAnnotationElementType,
        InvalidClassMember,
        InvalidSwitchLabel,

        UnsupportedParameterDefault,
        UnsupportedMemberTypeDeclaration,
        UnsupportedFieldInitializer,
        UnsupportedStatement,
        UnsupportedExpression,
        UnsupportedElementExpression,

        InterfaceFieldRequiresInitializer,
        ConstructorNameMismatch,

        StaticInitializerInMemberClass,

        ArrowColonMixUseInSwitch,
    };

    class parser_exception : public std::logic_error
    {
    public:
        const ParserExceptionType exception;
        const std::optional<Token> actualToken = std::nullopt;
        const std::optional<SourceRange> range = std::nullopt;

        parser_exception(ParserExceptionType exception)
            : logic_error(""), exception(exception)
        {

        }

        parser_exception(ParserExceptionType exception, std::optional<SourceRange> range)
            : logic_error(""), exception(exception), range(range)
        {

        }

        parser_exception(ParserExceptionType exception, Token actualToken)
            : logic_error(""), exception(exception), actualToken(actualToken), range(actualToken.range)
        {

        }

        parser_exception(ParserExceptionType exception, Token actualToken, const std::string& cause)
            : logic_error(cause), exception(exception), actualToken(actualToken), range(actualToken.range)
        {

        }
    };
}

#endif //CPPJAVAC_PARSEREXCEPTION_H
