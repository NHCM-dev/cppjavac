//
// Created by NHCM on 2026/11/25.
//

#ifndef CPPJAVAC_PARSEREXCEPTION_H
#define CPPJAVAC_PARSEREXCEPTION_H

#endif //CPPJAVAC_PARSEREXCEPTION_H

namespace javac
{
    enum class ParserExceptionType : uint8_t
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

        InvalidNumberLiteral,
        OutOfRangeNumberLiteral,

        KeywordInTypeSpecification,

        TypeArgUnboundUndefinedBehaviour,

        CannotExtendsTo,
        CannotBeSuper,
        CannotImplementsTo,
        CannotPermitsTo,
        InvalidExactGeneric,
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