//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_JAVACOMPILER_H
#define JAVACOMPILER_JAVACOMPILER_H

#include <string>
#include <vector>

#include "core/JavacConfig.h"

namespace javac
{
    class Compiler;

    inline std::string program = "javac";

    inline Compiler* compiler = nullptr;

    inline std::vector<std::string> args;
    inline JavacConfig* config = nullptr;

    inline uint64_t startTime = 0;
    inline uint64_t endTime = 0;

    inline std::string version =
R"(
cppjavac for Java 8
Version 0.0.2
Build b0.2
Date 2026-11-23

)";

    inline std::string helpDoc =
R"(
cppjavac

Usage: javac <options> [source files]

Required options:

    -c, --compile                  Compile source files into class files
    -i, --inspect                  Check for errors and warnings without
                                   generating output
    -l, --lex                      Test lexer functionality and write tokens
                                   result (experimental)

Options:

    -ex, --extension:{<name[,name...]>:all}
                                   Enable compiler extensions
    -s, --source-path <path>       Set the source file directory
    -o, --output-path <path>       Set the class file output directory
    -cp, --class-path <path>       Set the classpath directory
    -g, --generate-debug:{lines,vars,source:none}
                                   Generate debug information
    -h, --help                     Print this help message
    -v, --verbose                  Print debug information while compiling

    --list-extensions              List available compiler extensions
    --source <language level>      Set the source language level
    --target <language level>      Set the target class file level
    --encoding <encode>            Set the source file encoding
    --nowarn                       Suppress compiler warnings
    --version                      Print the compiler version

Arguments:

    [source files]                 Source files to compile or inspect

)";
}

#endif //JAVACOMPILER_JAVACOMPILER_H
