//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_JAVACCONFIG_H
#define JAVACOMPILER_JAVACCONFIG_H

#include "CoreDef.h"

namespace javac
{
    struct JavacConfig
    {
        // Input & Output
        std::vector<Path> srcFiles;
        std::vector<Path> srcPaths;
        Path outputDir;

        // Classpath
        std::vector<Path> classPaths;

        TextEncoding encoding; // Specify encoding of source files
        std::set<javac::DebugInfo> debugs; // output debug information options

        bool verbose; // Print debug information during compile or no
        bool nowarn; // Disable warning

    };
}

#endif //JAVACOMPILER_JAVACCONFIG_H
