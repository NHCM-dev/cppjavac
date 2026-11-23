//
// Created by NHCM on 2026/11/22.
//

#include "Compiler.h"

#include <cstdio>

#include "JavaCompiler.h"
#include "core/JavacConfig.h"
#include "lexer/Lexer.h"
#include "tool/EncodingTool.h"
#include "tool/FileIoTool.h"
#include "util/FileUtil.h"
#include "util/MathUtil.h"
#include "util/PrintUtil.h"

namespace javac
{
    Compiler::Compiler()
        : compilerConfig(*config)
    {
        compileRtData = new CompileRtData();
    }

    void Compiler::printTerminate()
    {
        jcutils::printlnErrorBy("Compiler terminated. " +
                std::to_string(compileRtData->errors.size()) + " errors occurred, " +
                std::to_string(compileRtData->warnings.size()) + " warnings generated.");
    }

    void Compiler::noticeError(const std::string& error)
    {
        compileRtData->errors.push_back(error);
        jcutils::printlnErrorBy(error);
    }
    void Compiler::noticeWarning(const std::string& warning)
    {
        if (!compilerConfig.nowarn)
        {
            compileRtData->warnings.push_back(warning);
            jcutils::printlnWarningBy(warning);
        }
    }

    bool Compiler::addParentPath(const Path& sourcePath, const Path& parentPath)
    {
        return compileRtData->addParentPath(sourcePath, parentPath);
    }

    Path* Compiler::getParentPath(const Path& sourceFilePath)
    {
        return compileRtData->getParentPath(sourceFilePath);
    }

    Path& Compiler::outputPath()
    {
        return compilerConfig.outputDir;
    }

    bool Compiler::putFileToWrite(const Path& filePath, const ByteArray& data)
    {
        return compileRtData->filesToWrite.try_emplace(fs::absolute(filePath), data).second;
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
        jcutils::printlnWarningBy("This lex feature is currently just an experiment");
        for (const auto& file : compileRtData->filesToCompile)
        {
            std::string str = FileIOTool::readUtf8(file);
            std::u16string u16str = EncodingTool::utf8ToUtf16(str);
            Lexer lexer = Lexer(u16str);

            if (verbose())
            {
                jcutils::printlnDebugBy("Lexing file \"" + file.string() + "\"");
            }

            std::vector<Token> tokens = lexer.lex(true, true);

            std::u16string sourceDisplay = {};
            std::string tokenDisplay {};

            for (const auto& token : tokens)
            {
                sourceDisplay+=token.content;

                if (token.tokenType != TokenType::Whitespace && token.tokenType != TokenType::NewLine)
                {
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
                                    char escape[7];
                                    std::snprintf(escape, sizeof(escape), "\\u%04X", static_cast<unsigned>(ch));
                                    content += escape;
                                }
                        }
                    }
                    std::string output = std::to_string(token.location.line) + ':'
                        + std::to_string(token.location.column) + ' ';
                    output += typeNames[static_cast<std::size_t>(token.tokenType)];
                    output += " \"" + content + '"';
                    tokenDisplay += output + "\n";
                }
            }
            sourceDisplay += u"\n\n===Lexer tokens===\n\n";
            sourceDisplay += EncodingTool::utf8ToUtf16(tokenDisplay);

            Path writePath = jcutils::joinAbsolutePath(outputPath(), file.filename());
            Path* parentPath = getParentPath(file);
            if (parentPath)
            {
                writePath = jcutils::joinAbsolutePath(outputPath(), jcutils::getRelativePath(file, *parentPath));
            }
            putFileToWrite(writePath, std::vector(
            reinterpret_cast<const std::byte*>(sourceDisplay.data()),
            reinterpret_cast<const std::byte*>(sourceDisplay.data() + sourceDisplay.size())
            ));
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
                noticeError("File or directory not found: " + absolutePath.string());
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
                                addParentPath(entryAbsolutePath, absolutePath);
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

    bool Compiler::postProcess()
    {
        bool error = false;
        for (const auto& path2Bytes : compileRtData->filesToWrite)
        {
            int result = FileIOTool::writeFile(path2Bytes.first, path2Bytes.second);
            switch (result)
            {
                case -2:
                    error = true;
                    noticeError("Failed to open or create file \"" + path2Bytes.first.string() + "\"");
                    break;
                case -1:
                    error = true;
                    noticeError("Failed to write to file \"" + path2Bytes.first.string() + "\"");
                    break;
                case 0:
                    if (verbose())
                    {
                        jcutils::printlnDebugBy("Successfully saved file \"" + path2Bytes.first.string() + "\"");
                    }
                    break;
                default:
                    error = true;
                    noticeError("Unknown error occurred when trying to writing to file \"" + path2Bytes.first.string() + "\"");
                    break;
            }
        }
        if (error)
        {
            return false;
        }
        if (verbose())
        {
            jcutils::printlnInfoBy("Saved " + std::to_string(compileRtData->filesToWrite.size()) + " file(s)");
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
            printTerminate();
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

        bool post = postProcess();
        if (!post)
        {
            printTerminate();
            return -1;
        }

        endTime = jcutils::getMSTime();

        jcutils::printlnInfoBy("Finished compiling, time took: " + std::to_string((endTime - startTime)) + "ms");

        return exitCode;
    }
} // javac
