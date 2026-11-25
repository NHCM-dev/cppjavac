//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_PARSER_H
#define JAVACOMPILER_PARSER_H
#include <utility>
#include <vector>

#include "main/ast/astdefs/Declarations.h"
#include "main/lexer/LexerDef.h"

namespace javac
{
    class Parser
    {
        const Path filePath;
        TokenStream inputStream;

        static CompilationUnitType decideType(const Path& filePath);

        CompilationUnit parseModular();
        CompilationUnit parseOrdinary();

    public:
        Parser(TokenStream inputStream, Path filePath)
        : filePath(std::move(filePath))
        {
            this->inputStream = std::move(inputStream);
            jcutils::removeIf(this->inputStream.tokens, [](const Token& token)
            {
                return token.tokenType == TokenType::Whitespace || token.tokenType == TokenType::NewLine;
            });
        }

        CompilationUnit parse();
    };

} // javac

#endif //JAVACOMPILER_PARSER_H
