//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_LEXER_H
#define JAVACOMPILER_LEXER_H

#include "LexerDef.h"
#include "main/core/def/Extension.h"

#include <cstddef>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace javac
{
    class Lexer
    {
        std::u16string sourceCode;
        std::set<JavacExtension> extensions;
        std::vector<Token> tokens;
        std::u16string tokenBuffer;

        std::size_t offset = 0;
        std::size_t line = 1;
        std::size_t column = 0;
        SourceLocation tokenStart;

        bool needEndString = false;
        bool needEndChar = false;
        bool needEndComment = false;
        bool isEscaping = false;

        TokenType needEndCommentType = TokenType::Invalid;

        // Compiler extensions
        bool keywordIdentifiers;
        bool numericStart;
        LangLevel sourceLangLevel;

        void pushToken(TokenType type,
            std::optional<SymbolType> symbol = std::nullopt,
            std::optional<JavaKeyword> keyword = std::nullopt,
            std::optional<InvalidTokenType> invalidReason = std::nullopt,
            bool canBeIdentifier = false);

        std::span<const char16_t> readNextX(std::size_t startOffset, std::size_t length);

    public:
        Lexer(std::u16string_view sourceCode, std::set<JavacExtension> extensions = {},
            LangLevel sourceLangLevel = LangLevel::Eight);

        // Include comment tokens when comment is true. Unterminated comments always produce Invalid.
        std::vector<Token> lex(bool spaceAndLine = false, bool comment = false);
    };
} // javac

#endif //JAVACOMPILER_LEXER_H
