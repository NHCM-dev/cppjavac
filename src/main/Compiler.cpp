//
// Created by NHCM on 2026/11/22.
//

#include "Compiler.h"

#include <cstdio>
#include <stdexcept>

#include "cppjavac.h"
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
        jcutils::printlnErrorCustom("Compiler terminated. " +
                std::to_string(compileRtData->errors.size()) + " errors occurred, " +
                std::to_string(compileRtData->warnings.size()) + " warnings generated.");
    }

    void Compiler::noticeError(const std::string& error, std::string_view owner)
    {
        compileRtData->errors.push_back(error);
        jcutils::printlnErrorCustom(error, owner);
    }
    void Compiler::noticeWarning(const std::string& warning)
    {
        if (!compilerConfig.nowarn)
        {
            compileRtData->warnings.push_back(warning);
            jcutils::printlnWarningCustom(warning);
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

    TextEncoding& Compiler::encoding()
    {
        return compilerConfig.encoding;
    }

    bool Compiler::putFileToWrite(const Path& filePath, const ByteArray& data)
    {
        return compileRtData->filesToWrite.try_emplace(fs::absolute(filePath), data).second;
    }

    Path Compiler::getWritePath(const Path& filePath)
    {
        Path writePath = jcutils::joinAbsolutePath(outputPath(), filePath.filename());
        Path* parentPath = getParentPath(filePath);
        if (parentPath)
        {
            writePath = jcutils::joinAbsolutePath(outputPath(), jcutils::getRelativePath(filePath, *parentPath));
        }
        return writePath;
    }
    Path Compiler::getWritePath(const Path& filePath, const std::string& newExtension)
    {
        return jcutils::changeExtension(getWritePath(filePath), newExtension);
    }

    int Compiler::compile()
    {
        jcutils::printlnErrorCustom("Compile is not implemented");
        return -1;
    }
    int Compiler::inspect()
    {
        jcutils::printlnErrorCustom("Inspect is not implemented");
        return -1;
    }
    int Compiler::lex()
    {
        jcutils::printlnWarningCustom("This lex feature is currently experimental", LEXER);
        bool hasErrors = false;
        for (const auto& file : compileRtData->filesToCompile)
        {
            const std::string displayPath = file.string();

            if (verbose())
            {
                jcutils::printlnDebugCustom("Lexing file \"" + displayPath + "\"", LEXER);
            }

            std::u16string sourceDisplay;
            std::vector<Token> tokens;
            try
            {
                sourceDisplay = EncodingTool::decodeToUtf16(FileIOTool::readUtf8(file), encoding());
                Lexer lexer(sourceDisplay, compilerConfig.extensions, compilerConfig.sourceLangLevel);
                tokens = lexer.lex(true, true);
            }
            catch (const std::invalid_argument& error)
            {
                noticeError(displayPath + ": " + error.what(), LEXER);
                hasErrors = true;
                continue;
            }
            catch (const std::ios_base::failure& error)
            {
                noticeError(error.what(), LEXER);
                hasErrors = true;
                continue;
            }
            std::string tokenDisplay {};

            for (const auto& token : tokens)
            {
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
                    const std::string position = std::to_string(token.location.line) + ':'
                        + std::to_string(token.location.column);
                    std::string output = position + ' ';
                    output += typeNames[static_cast<std::size_t>(token.tokenType)];
                    output += " \"" + content + '"';
                    if (token.invalidReason)
                    {
                        constexpr std::string_view reasons[] = {
                            "Line break in string literal", "Line break in character literal",
                            "Unterminated escape in string literal", "Unterminated escape in character literal",
                            "Unterminated string literal", "Unterminated character literal", "Unterminated block comment",
                            "Too many characters in character literal", "Unrecognized token", "Empty character literal",
                            "Invalid escape sequence", "Invalid numeric literal"
                        };
                        const std::string reason(reasons[static_cast<std::size_t>(*token.invalidReason)]);
                        noticeError(displayPath + ':' + position + ": " + reason, LEXER);
                        output += " (" + reason + ')';
                        hasErrors = true;
                    }
                    tokenDisplay += output + "\n";
                }
            }
            sourceDisplay += u"\n\n===Lexer tokens===\n\n";
            sourceDisplay += EncodingTool::utf8ToUtf16(tokenDisplay);

            putFileToWrite(getWritePath(file, "javal"), std::vector(
            reinterpret_cast<const std::byte*>(sourceDisplay.data()),
            reinterpret_cast<const std::byte*>(sourceDisplay.data() + sourceDisplay.size())
            ));
        }

        return hasErrors ? -1 : 0;
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
                            jcutils::printlnDebugCustom("Adding file \"" + absolutePath.string() + "\"");
                        }
                    }
                } else if (is_directory(absolutePath))
                {
                    if (verbose())
                    {
                        jcutils::printlnDebugCustom("Scanning directory \"" + absolutePath.string() + "\"");
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
                                    jcutils::printlnDebugCustom("Adding file \"" + entryAbsolutePath.string() + "\"");
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
            jcutils::printlnInfoCustom("Scanned and found " + std::to_string(compileRtData->filesToCompile.size()) + " file(s) to compile");
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
                        jcutils::printlnDebugCustom("Successfully saved file \"" + path2Bytes.first.string() + "\"");
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
            jcutils::printlnInfoCustom("Saved " + std::to_string(compileRtData->filesToWrite.size()) + " file(s)");
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
                jcutils::printlnErrorCustom("Unknown compiler starting action, this should not happen");
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

        jcutils::printlnInfoCustom("Finished compiling, time took: " + std::to_string((endTime - startTime)) + "ms");

        return exitCode;
    }
} // javac
