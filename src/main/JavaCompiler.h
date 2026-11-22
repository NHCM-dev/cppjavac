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
    inline std::string program = "javac";

    inline std::vector<std::string> args;
    inline JavacConfig config;

    inline std::string version =
R"(
javac (Simple Java Compiler) for Java 8
Version 0.0.1
Build b0.1
Date 2026-11-22

)";

    inline std::string helpDoc =
R"(
Usage: javac <options> [source files]

Required options:

    -c, --compile                                   Compile input source files into class files and generate output
    -i, --inspect                                   Only test errors and warning of compiling, without generating output

Options:

    -s, --source-path <path>                        Specify source file input directory
    -o, --output-path <path>                        Specify class file output directory directory
    -cp, --class-path <path>                        Specify the use of classpath directory when compiling
    -g, --generate-debug:{lines,vars,source:none}   Generate debug information
    -h, --help                                      Print this help message
    -v, --verbose                                   Print debug information when compiling

    --source <language level>                       Specify source file language level capability
    --target <language level>                       Specify output class file language level capability
    --encoding <encode>                             Specify source file text encoding
    --nowarn                                        Disable warning when compiling
    --version                                       Print current program version

    [source files]                                   Specify source file inputs
)";
}

int main(int argc, char** argv);

#endif //JAVACOMPILER_JAVACOMPILER_H
