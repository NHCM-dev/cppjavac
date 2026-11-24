//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_LEXERDEF_H
#define JAVACOMPILER_LEXERDEF_H
#include <optional>
#include <string>
#include <string_view>

#include "main/core/DataDef.h"
#include "main/core/def/JavaDef.h"

namespace javac
{

    enum class TokenType
    {
        // Special
        EndOfFile,             // End of source file
        Invalid,               // Invalid token

        // Identifiers & Keywords
        Identifier,            // Variable, class, method names
        Keyword,               // Java keyword

        // Literals
        IntegerLiteral,        // 123, 0xFF, 123L
        FloatLiteral,          // 3.14, 1.0f, 1e10, 1.0d, 1e10d
        CharacterLiteral,      // 'a', '\n'
        StringLiteral,         // "Hello"

        // Symbols
        Symbol,                // Operators and separators

        // Comments
        LineComment,           // // ...
        BlockComment,          // /* ... */
        JavadocComment,        // /** ... */

        // Format
        NewLine,               // \n
        Whitespace,            // ' '

        BooleanLiteral,        // true, false
        NullLiteral,           // null
    };


    enum class SymbolType
    {
        // Separators
        LParen,                    // (
        RParen,                    // )
        LBrace,                    // {
        RBrace,                    // }
        LBracket,                  // [
        RBracket,                  // ]

        Question,                  // ?
        Colon,                     // :

        Semicolon,                 // ;
        Comma,                     // ,
        Dot,                       // .
        Ellipsis,                  // ...

        AtSign,                    // @
        DoubleColon,               // ::

        Underscore,                // _

        // Arithmetic Operators
        Plus,                      // +
        Minus,                     // -
        Star,                      // *
        Slash,                     // /
        Percent,                   // %
        PlusPlus,                  // ++
        MinusMinus,                // --

        // Assignment Operators
        Assign,                    // =
        PlusAssign,                // +=
        MinusAssign,               // -=
        StarAssign,                // *=
        SlashAssign,               // /=
        PercentAssign,             // %=

        AmpersandAssign,           // &=
        PipeAssign,                // |=
        CaretAssign,               // ^=
        LeftShiftAssign,           // <<=
        RightShiftAssign,          // >>=
        UnsignedRightShiftAssign,  // >>>=

        // Comparison Operators
        EqualEqual,                // ==
        BangEqual,                 // !=
        LeftAngleBracket,          // <
        LessEqual,                 // <=
        RightAngleBracket,         // >
        GreaterEqual,              // >=

        // Logical / Bitwise Operators
        Bang,                      // !
        AmpersandAmpersand,        // &&
        PipePipe,                  // ||
        Ampersand,                 // &
        Pipe,                      // |
        Caret,                     // ^
        Tilde,                     // ~
        LeftShift,                 // <<
        RightShift,                // >>
        UnsignedRightShift,        // >>>

        // Other Operators
        Arrow,                     // ->
    };

    // The entire spelling must match one symbol.
    [[nodiscard]] std::optional<SymbolType> findSymbol(std::string_view spelling);
    [[nodiscard]] std::optional<SymbolType> findSymbol(std::u16string_view spelling);
    [[nodiscard]] bool isSymbol(std::string_view spelling, SymbolType expected);
    [[nodiscard]] bool isSymbol(std::u16string_view spelling, SymbolType expected);

    // True when this symbol cannot overlap a symbol that extends beyond either edge.
    [[nodiscard]] bool isStandaloneSymbol(std::string_view spelling);
    [[nodiscard]] bool isStandaloneSymbol(std::u16string_view spelling);

    enum class InvalidTokenType
    {
        SwitchLineInStringLiteral,
        SwitchLineInCharacterLiteral,

        NoneFinishEscapeInStringLiteral,
        NoneFinishEscapeInCharacterLiteral,

        NoneFinishStringLiteral,
        NoneFinishCharacterLiteral,
        NoneFinishMultiLineComment,

        TooLargeCharacterLiteral,

        UnknownTokenType,
        EmptyCharacterLiteral,
        InvalidEscapeSequence,
        InvalidNumericLiteral
    };

    struct Token
    {
        TokenType tokenType;
        std::u16string content;
        SourceLocation location;

        std::optional<SymbolType> symbol;
        std::optional<JavaKeyword> keyword;
        std::optional<InvalidTokenType> invalidReason;
        bool canBeIdentifier = false; // The parser may consume this token in a name position.
    };

} // javac

#endif //JAVACOMPILER_LEXERDEF_H
