//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_LEXER_H
#define JAVACOMPILER_LEXER_H

#include "LexerDef.h"
#include "main/core/def/Extension.h"
#include <stdexcept>

#include <cstddef>
#include <optional>
#include <set>
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
        std::vector<std::unique_ptr<Token>> invalidTokens;
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

    public:
        Lexer(std::u16string_view sourceCode, std::set<JavacExtension> extensions = {},
            LangLevel sourceLangLevel = LangLevel::Eight);

        /**
         * Tokenize java source code and generate a struct contains vector of tokens and references of invalid tokens
         * @throws std::invalid_argument for malformed Unicode escapes before tokenization
         * @param spaceAndLine Process and produce space and new line tokens
         * @param comment Process and produce comment tokens
         * @return return a vector of tokens
         */
        TokenStream lex(bool spaceAndLine = false, bool comment = false);
    };
} // javac

#endif //JAVACOMPILER_LEXER_H
