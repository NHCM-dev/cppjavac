//
// Created by NHCM on 2026/11/22.
//

#include "Compiler.h"

#include "JavaCompiler.h"
#include "core/JavacConfig.h"
#include "util/MathUtil.h"
#include "util/PrintUtil.h"

namespace javac
{
    int Compiler::compile()
    {
        jcutils::printlnErrorBy("Compile is not implemented");
        return -1;
    }
    int Compiler::inspect()
    {
        jcutils::printlnErrorBy("Inspect is not implemented");
        return -1;
    }
    int Compiler::lex()
    {
        jcutils::printlnErrorBy("Lex is not implemented");
        return -1;
    }

    void Compiler::preProcess()
    {

    }

    int Compiler::run()
    {
        startTime = jcutils::getMSTime();

        int exitCode = 0;

        preProcess();

        switch (compilerConfig.action)
        {
            case JavacAction::Compile:
            {
                exitCode = compile();
                break;
            }
            case JavacAction::Inspect:
            {
                exitCode = inspect();
                break;
            }
            case JavacAction::Lexer:
            {
                exitCode = lex();
                break;
            }
            default:
            {
                jcutils::printlnErrorBy("Unknown compiler starting action, this should not happen");
                return -1;
            }
        }

        endTime = jcutils::getMSTime();

        jcutils::printlnInfoBy("Finished compiling, time took: " + std::to_string((endTime - startTime)) + "ms");

        return exitCode;
    }
} // javac