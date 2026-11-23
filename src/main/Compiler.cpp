//
// Created by NHCM on 2026/11/22.
//

#include "Compiler.h"

#include <format>

#include "JavaCompiler.h"
#include "core/JavacConfig.h"
#include "lexer/Lexer.h"
#include "tool/EncodingTool.h"
#include "tool/FileIoTool.h"
#include "util/MathUtil.h"
#include "util/PrintUtil.h"

namespace javac
{
    Compiler::Compiler()
        : compilerConfig(*config)
    {
        compileRtData = new CompileRtData();
    }

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
        jcutils::printlnWarningBy("This feature is currently a test");
        for (const auto& file : compileRtData->filesToCompile)
        {
            std::string str = FileIOTool::readUtf8(file);
            std::u16string u16str = EncodingTool::utf8ToUtf16(str);
            Lexer lexer = Lexer(u16str);
            std::vector<Token> tokens = lexer.lex();
            for (const auto& token : tokens)
            {
                // Match the order of TokenType in LexerDef.h.
                constexpr std::string_view typeNames[] = {
                    "EndOfFile", "Invalid", "Identifier", "Keyword",
                    "IntegerLiteral", "FloatLiteral", "CharacterLiteral", "StringLiteral",
                    "Symbol", "LineComment", "BlockComment", "JavadocComment",
                    "NewLine", "Whitespace", "BooleanLiteral", "NullLiteral"
                };

                std::string content;
                for (const char16_t ch : token.content)
                {
                    switch (ch)
                    {
                        case u'\n': content += "\\n"; break;
                        case u'\r': content += "\\r"; break;
                        case u'\t': content += "\\t"; break;
                        case u'"': content += "\\\""; break;
                        case u'\\': content += "\\\\"; break;
                        default:
                            if (ch >= u' ' && ch <= u'~')
                            {
                                content += static_cast<char>(ch);
                            }
                            else
                            {
                                content += std::format("\\u{:04X}", static_cast<unsigned>(ch));
                            }
                    }
                }
                jcutils::println(std::format("{}:{} {} \"{}\"", token.location.line,
                    token.location.column, typeNames[static_cast<std::size_t>(token.tokenType)], content),
                    jcutils::PrintColor::None);
            }
        }
        return 0;
    }

    bool Compiler::verbose() const
    {
        return compilerConfig.verbose;
    }

    bool Compiler::preProcess()
    {
        bool error = false;
        for (const Path& path : compilerConfig.srcPaths)
        {
            Path absolutePath = absolute(path);
            if (!exists(absolutePath))
            {
                std::string notFound = "File or directory not found: " + absolutePath.string();
                compileRtData->errors.push_back(notFound);
                jcutils::printlnErrorBy(notFound);
                error = true;
            } else
            {
                if (is_regular_file(absolutePath) && absolutePath.extension() == ".java")
                {
                    if (jcutils::containsOrInsert(compileRtData->filesToCompile, absolutePath))
                    {
                        if (verbose())
                        {
                            jcutils::printlnDebugBy("Adding file \"" + absolutePath.string() + "\"");
                        }
                    }
                } else if (is_directory(absolutePath))
                {
                    if (verbose())
                    {
                        jcutils::printlnDebugBy("Scanning directory \"" + absolutePath.string() + "\"");
                    }
                    for (const auto& entry : fs::recursive_directory_iterator(absolutePath))
                    {
                        Path entryAbsolutePath = absolute(entry.path());
                        if (is_regular_file(entryAbsolutePath) && entryAbsolutePath.extension() == ".java")
                        {
                            if (jcutils::containsOrInsert(compileRtData->filesToCompile, entryAbsolutePath))
                            {
                                if (verbose())
                                {
                                    jcutils::printlnDebugBy("Adding file \"" + entryAbsolutePath.string() + "\"");
                                }
                            }
                        }
                    }
                }
            }
        }
        if (error)
        {
            return false;
        }
        if (verbose())
        {
            jcutils::printlnInfoBy("Scanned and found " + std::to_string(compileRtData->filesToCompile.size()) + " file(s) to compile");
        }
        return true;
    }

    int Compiler::run()
    {
        startTime = jcutils::getMSTime();

        int exitCode = 0;

        bool pre = preProcess();
        if (!pre)
        {
            jcutils::printlnErrorBy("Compiler terminated. " +
                std::to_string(compileRtData->errors.size()) + " errors occurred, " +
                std::to_string(compileRtData->warnings.size()) + " warnings generated.");
            return -1;
        }

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
