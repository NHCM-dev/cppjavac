//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_PARSER_H
#define JAVACOMPILER_PARSER_H
#include <cstddef>
#include <deque>
#include <queue>
#include <stack>
#include <utility>
#include <vector>

#include "main/ast/astdefs/Declarations.h"
#include "main/lexer/LexerDef.h"

namespace javac
{
    namespace
    {
        constexpr uint8_t S_PACKAGE = 0;
        constexpr uint8_t S_ANNOTATION = 1;
    }

    enum class ParserOp : uint8_t
    {
        None,
        PendingAnnotation,
        PendingPackage,
        PendingSemiColon,
    };

    /**
     * Class that parse a valid stream of tokens and generate AST tree
     */
    class Parser
    {
        const Path filePath;
        TokenStream inputStream;

        CompilationUnit compilationUnit = {};

        ParserOp op = ParserOp::None;

        std::array<std::u16string, 256> stringBufferSlots = {};
        std::deque<Annotation> annotationBuffer;

        // Opening delimiters that are still waiting for a match.
        std::size_t unusedParen = 0;
        std::size_t unusedBracket = 0;
        std::size_t unusedBrace = 0;
        std::stack<SymbolType> delimiterOrder;

        // === basic tools ===
        void errorProcessPlaceholder();

        static CompilationUnitType decideType(const Path& filePath);

        bool isOp(ParserOp operation);
        void setOp(ParserOp operation);
        void clearOp();
        bool opUnfinished();

        void appendBuffer(uint8_t slot, std::u16string_view string);
        void appendBuffer(uint8_t slot, char16_t character);
        const std::u16string& readBuffer(uint8_t slot);
        std::u16string flushBuffer(uint8_t slot);
        //bool bufferHasSpace(uint8_t slot);

        void pushAnnotation(Annotation annotation);
        std::optional<Annotation> popAnnotation();
        std::queue<Annotation> flushAnnotation();

        void pushDelimiter(SymbolType opening);
        void popDelimiter(SymbolType opening);
        void clearDelimiters();
        std::size_t delimiterCount(SymbolType type);
        std::size_t& delimiterCounter(SymbolType opening);

        PackageDecl& getOrCreatePackage();
        // ======



        // === parsing logics ===
        void parseModular();
        void parseOrdinary();

        void processWhitespace();
        void processKeywords(const Token& token);
        void processIdentifier(const Token& token);
        void processSymbol(const Token& token);
        // ======


    public:
        /**
         * @throws std::invalid_argument if the given token streams contains invalid tokens
         * @param inputStream Token streams which contains no invalid tokens
         * @param filePath path of source file
         */
        Parser(TokenStream inputStream, Path filePath);

        CompilationUnit parse();
    };

} // javac

#endif //JAVACOMPILER_PARSER_H
