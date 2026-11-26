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
#include "util/ExceptionUtil.h"
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
        jcutils::printErrorCustom("Compiler terminated. " +
                std::to_string(compileRtData->errors.size()) + " errors occurred, " +
                std::to_string(compileRtData->warnings.size()) + " warnings generated.");
    }

    void Compiler::noticeError(const std::string& error, std::string_view owner)
    {
        compileRtData->errors.push_back(error);
        jcutils::printErrorCustom(error, owner);
    }
    void Compiler::noticeWarning(const std::string& warning)
    {
        if (!compilerConfig.nowarn)
        {
            compileRtData->warnings.push_back(warning);
            jcutils::printWarningCustom(warning);
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

    bool Compiler::compile()
    {
        jcutils::printErrorCustom("Compile is not implemented");
        return -1;
    }
    bool Compiler::inspect()
    {
        jcutils::printErrorCustom("Inspect is not implemented");
        return -1;
    }

    bool Compiler::lex()
    {
        bool hasErrors = false;
        for (const auto& file : compileRtData->filesToCompile)
        {
            const std::string displayPath = file.string();

            if (verbose())
            {
                jcutils::printDebugCustom("Lexing file \"" + displayPath + "\"", LEXER);
            }

            std::u16string sourceDisplay;
            TokenStream tokenStream;
            try
            {
                sourceDisplay = EncodingTool::decodeToUtf16(FileIOTool::readUtf8(file), encoding());
                Lexer lexer(sourceDisplay, compilerConfig.extensions, compilerConfig.sourceLangLevel);
                tokenStream = lexer.lex();
            }
            catch (const std::invalid_argument& error)
            {
                noticeError(displayPath + ": " + error.what());
                hasErrors = true;
                continue;
            }
            catch (const std::ios_base::failure& error)
            {
                noticeError(error.what(), LEXER);
                hasErrors = true;
                continue;
            }

            hasErrors |= reportInvalidTokens(file, sourceDisplay, tokenStream);

            compileRtData->tokenStreams.emplace(file, std::move(tokenStream));
        }

        return !hasErrors;
    }

    bool Compiler::reportInvalidTokens(const Path& file, std::u16string_view source, const TokenStream& tokens)
    {
        for (const auto& token : tokens.invalidTokens)
        {
            const auto message = invalidTokenStr(token->invalidReason.value());
            jcutils::printDiagnostic(jcutils::DiagnosticSeverity::Error, file, message,
                token->range, source);
            compileRtData->errors.emplace_back(message);
        }
        return !tokens.invalidTokens.empty();
    }

    bool Compiler::lexExperimental()
    {
        jcutils::printWarningCustom("This lex feature is currently experimental", LEXER);
        bool hasErrors = false;
        for (const auto& file : compileRtData->filesToCompile)
        {
            const std::string displayPath = file.string();

            if (verbose())
            {
                jcutils::printDebugCustom("Lexing file \"" + displayPath + "\"", LEXER);
            }

            std::u16string sourceDisplay;
            TokenStream tokenStream;
            try
            {
                sourceDisplay = EncodingTool::decodeToUtf16(FileIOTool::readUtf8(file), encoding());
                Lexer lexer(sourceDisplay, compilerConfig.extensions, compilerConfig.sourceLangLevel);
                tokenStream = lexer.lex(true, true);
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
            hasErrors |= reportInvalidTokens(file, sourceDisplay, tokenStream);
            std::string tokenDisplay {};

            for (const auto& token : tokenStream.tokens)
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
                    const std::string position = sourceLocationToString(token.range.begin);
                    std::string output = position + ' ';
                    output += typeNames[static_cast<std::size_t>(token.tokenType)];
                    output += " \"" + content + '"';
                    if (token.invalidReason)
                    {
                        output += " (";
                        output += invalidTokenStr(*token.invalidReason);
                        output += ')';
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

        return !hasErrors;
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
                            jcutils::printDebugCustom("Adding file \"" + absolutePath.string() + "\"");
                        }
                    }
                } else if (is_directory(absolutePath))
                {
                    if (verbose())
                    {
                        jcutils::printDebugCustom("Scanning directory \"" + absolutePath.string() + "\"");
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
                                    jcutils::printDebugCustom("Adding file \"" + entryAbsolutePath.string() + "\"");
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
            jcutils::printInfoCustom("Scanned and found " + std::to_string(compileRtData->filesToCompile.size()) + " file(s) to compile", VERBOSE);
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
                        jcutils::printDebugCustom("Successfully saved file \"" + path2Bytes.first.string() + "\"");
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
            jcutils::printInfoCustom("Saved " + std::to_string(compileRtData->filesToWrite.size()) + " file(s)", VERBOSE);
        }
        return true;
    }

    bool Compiler::run()
    {
        startTime = jcutils::getMSTime();

        bool hasError = false;

        bool pre = preProcess();
        if (!pre)
        {
            printTerminate();
            return false;
        }

        switch (compilerConfig.action)
        {
            case JavacAction::Compile:
            {
                hasError = !compile();
                break;
            }
            case JavacAction::Inspect:
            {
                hasError = !inspect();
                break;
            }
            case JavacAction::Lexer:
            {
                hasError = !lexExperimental();
                break;
            }
            default:
            {
                jcutils::printErrorCustom("Unknown compiler starting action, this should not happen");
                return false;
            }
        }

        bool post = postProcess();
        if (!post)
        {
            printTerminate();
            return -1;
        }

        endTime = jcutils::getMSTime();

        jcutils::printInfoBy("Finished compiling, time took: " + std::to_string((endTime - startTime)) + "ms");

        return !hasError;
    }
} // javac
