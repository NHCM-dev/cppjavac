//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_COMPILER_H
#define JAVACOMPILER_COMPILER_H
#include <map>

#include "JavaCompiler.h"
#include "core/JavacConfig.h"
#include "lexer/LexerDef.h"

namespace javac
{
    struct CompileRtData
    {
        std::map<Path, std::vector<Token>> tokenStreams = {};

        std::vector<Path> filesToCompile = {};

        std::vector<Token>* getTokens(const Path& sourcePath)
        {
           return jcutils::getOrNull(tokenStreams, sourcePath);
        }
    };

    class Compiler
    {
        CompileRtData* data = nullptr;
        JavacConfig& compilerConfig;

        int compile();
        int inspect();
        int lex();
        void preProcess();
    public:
        Compiler()
        : compilerConfig(*config)
        {
            data = new CompileRtData();
        }
        ~Compiler()
        {
            free(data);
            delete data;
        }

        int run();
    };

} // javac

#endif //JAVACOMPILER_COMPILER_H
