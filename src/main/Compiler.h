//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_COMPILER_H
#define JAVACOMPILER_COMPILER_H
#include <map>

#include "core/JavacConfig.h"
#include "lexer/LexerDef.h"
#include "util/DataUtil.h"

namespace javac
{
    struct CompileRtData
    {
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        std::map<Path, std::vector<Token>> tokenStreams = {};
        std::set<Path> filesToCompile = {};

        std::vector<Token>* getTokens(const Path& sourcePath)
        {
           return jcutils::getOrNull(tokenStreams, sourcePath);
        }
    };

    class Compiler
    {
        CompileRtData* compileRtData = nullptr;
        JavacConfig& compilerConfig;

        int compile();
        int inspect();
        int lex();
        bool preProcess();

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
