//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_JAVACCONFIG_H
#define JAVACOMPILER_JAVACCONFIG_H

#include "DataDef.h"
#include "def/Extension.h"

namespace javac
{
    enum class JavacAction : uint8_t
    {
        Compile = 0,
        Inspect = 1,

        // Experimental actions
        Lexer = 100,
        Parser = 101,
    };

    struct JavacConfig
    {
        JavacAction action; // Action to do for this current program run

        // Input & Output
        std::vector<Path> srcPaths;
        Path outputDir;

        // Classpath
        std::vector<Path> classPaths;

        TextEncoding encoding; // Specify encoding of source files
        std::set<DebugInfo> debugs; // output debug information options

        LangLevel sourceLangLevel; // Input source file language level capability
        LangLevel targetLangLevel; // Output class file language level capability

        bool verbose; // Print debug information during compile or no
        bool nowarn; // Disable warning
        std::set<JavacExtension> extensions;

        JavacConfig(
            JavacAction action,
            std::vector<Path> srcPaths,
            Path outputDir,
            std::vector<Path> classPaths,
            TextEncoding encoding,
            std::set<DebugInfo> debugs,
            LangLevel sourceLangLevel,
            LangLevel targetLangLevel,
            bool verbose,
            bool nowarn,
            std::set<JavacExtension> extensions = {}
        )
        : action(action),
          srcPaths(std::move(srcPaths)),
          outputDir(std::move(outputDir)),
          classPaths(std::move(classPaths)),
          encoding(encoding),
          debugs(std::move(debugs)),
          sourceLangLevel(sourceLangLevel),
          targetLangLevel(targetLangLevel),
          verbose(verbose),
          nowarn(nowarn),
          extensions(std::move(extensions))
        {
        }

        [[nodiscard]] bool hasExtension(JavacExtension extension) const
        {
            return extensions.contains(extension);
        }
    };
}

#endif //JAVACOMPILER_JAVACCONFIG_H
