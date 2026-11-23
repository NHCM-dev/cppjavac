//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_COMPILER_H
#define JAVACOMPILER_COMPILER_H
#include <map>

#include "core/JavacConfig.h"
#include "lexer/LexerDef.h"
#include "util/DataUtil.h"
#include "util/FileUtil.h"

namespace javac
{
    struct CompileFile
    {
        CompileFile(const Path& path)
        : absolutePath(fs::absolute(path)), sourcePath(std::nullopt)
        {}

        CompileFile(const Path& path, const Path& srcPath)
       : absolutePath(fs::absolute(path)), sourcePath(srcPath)
        {}

        Path absolutePath;
        std::optional<Path> sourcePath;

        bool operator==(const CompileFile& another) const
        {
            return this->absolutePath == another.absolutePath;
        }
    };

    struct CompileRtData
    {
        std::vector<std::string> errors;
        std::vector<std::string> warnings;

        std::map<Path, Path> parentPathRecord = {};

        std::map<Path, std::vector<Token>> tokenStreams = {};

        std::set<Path> filesToCompile = {};
        std::map<Path, ByteArray> filesToWrite = {};

        std::vector<Token>* getTokens(const Path& sourcePath)
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

        void noticeError(const std::string& error);
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

        int compile();
        int inspect();
        int lex();
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

        int run();
    };

} // javac

#endif //JAVACOMPILER_COMPILER_H
