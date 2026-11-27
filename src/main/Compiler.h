//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_COMPILER_H
#define JAVACOMPILER_COMPILER_H
#include <map>

#include "core/JavacConfig.h"
#include "lexer/LexerDef.h"
#include "parser/Parser.h"
#include "parser/ast/astdefs/Declarations.h"
#include "util/DataUtil.h"

namespace javac
{
    inline constexpr std::string_view LEXER = "lexer";
    inline constexpr std::string_view PARSER = "parser";
    inline constexpr std::string_view VERBOSE = "verbose";

    struct CompileRtData
    {
        std::vector<std::string> errors;
        std::vector<std::string> warnings;

        std::map<Path, Path> parentPathRecord = {};

        std::map<Path, TokenStream> tokenStreams = {};
        std::map<Path, ParserResult> parserResults = {};

        std::map<Path, std::u16string> sourcesDisplay = {};

        std::set<Path> filesToCompile = {};
        std::map<Path, ByteArray> filesToWrite = {};

        TokenStream* getTokenStream(const Path& sourcePath)
        {
           return jcutils::getOrNull(tokenStreams, sourcePath);
        }

        Path* getParentPath(const Path& sourceFilePath)
        {
            return jcutils::getOrNull(parentPathRecord, sourceFilePath);
        }

        bool addParentPath(const Path& sourcePath, const Path& parentPath)
        {
            return parentPathRecord.try_emplace(sourcePath, parentPath).second;
        }
    };

    class Compiler
    {
        void printTerminate();

        void noticeError(const std::string& error, std::string_view owner = {});
        void noticeWarning(const std::string& warning);


        bool addParentPath(const Path& sourcePath, const Path& parentPath);
        Path* getParentPath(const Path& sourceFilePath);

        Path& outputPath();
        TextEncoding& encoding();

        bool putFileToWrite(const Path& filePath, const ByteArray& data);
        Path getWritePath(const Path& filePath);
        Path getWritePath(const Path& filePath, const std::string& newExtension);

        CompileRtData* compileRtData = nullptr;
        JavacConfig& compilerConfig;

        bool compile();
        bool inspect();

        bool lex();
        bool reportInvalidTokens(const Path& file, std::u16string_view source, const TokenStream& tokens);
        bool lexExperimental();

        bool parse();
        bool reportParsingErrorExperimental(const Path& file, std::u16string_view source,
            const ParserResult& parserResult);
        bool parseExperimental();

        bool preProcess();
        bool postProcess();

        [[nodiscard]]
        bool verbose() const;
    public:
        Compiler();

        ~Compiler()
        {
            delete compileRtData;
        }

        bool run();
    };

} // javac

#endif //JAVACOMPILER_COMPILER_H
