//
// Created by NHCM on 2026/11/22.
//

#include "Compiler.h"

#include <stdexcept>
#include <utility>

#include "cppjavac.h"
#include "core/JavacConfig.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "tool/EncodingTool.h"
#include "tool/FileIoTool.h"
#include "util/ExceptionUtil.h"
#include "util/MathUtil.h"
#include "util/PrintUtil.h"

namespace javac
{
    Compiler::Compiler()
        : compilerConfig(*config)
    {
        compileRtData = new CompileRtData();
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
        bool hasError = false;
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
                noticeError(displayPath + ": " + error.what(), LEXER);
                hasError = true;
                continue;
            }
            catch (const std::ios_base::failure& error)
            {
                noticeError(error.what(), LEXER);
                hasError = true;
                continue;
            }

            hasError |= reportInvalidTokens(file, sourceDisplay, tokenStream);

            compileRtData->sourcesDisplay.emplace(file, std::move(sourceDisplay));
            compileRtData->tokenStreams.emplace(file, std::move(tokenStream));
        }

        return !hasError;
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

    bool Compiler::parse()
    {
        bool hasError = false;
        for (auto& [file, tokenStream] : compileRtData->tokenStreams)
        {
            if (!tokenStream.invalidTokens.empty())
            {
                hasError = true;
                continue;
            }

            const std::string displayPath = file.string();

            if (verbose())
            {
                jcutils::printDebugCustom("Parsing file \"" + displayPath + "\"", PARSER);
            }

            Parser parser = {std::move(tokenStream), file, compilerConfig.extensions, compilerConfig.sourceLangLevel};

            ParserResult result;
            try
            {
                result = parser.parse();
            } catch (const parser_exception& error)
            {
                result.exceptions.push_back(error);
            }

            hasError |= reportParsingErrorExperimental(file, compileRtData->sourcesDisplay.at(file), result);

            compileRtData->parserResults.emplace(file, std::move(result));
        }
        return !hasError;
    }

    bool Compiler::preProcess()
    {
        bool hasError = false;
        for (const Path& path : compilerConfig.srcPaths)
        {
            Path absolutePath = absolute(path);
            if (!exists(absolutePath))
            {
                noticeError("File or directory not found: " + absolutePath.string());
                hasError = true;
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
        if (hasError)
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
        size_t errorCount = compileRtData->errors.size();
        size_t warningCount = compileRtData->warnings.size();
        std::string errorAndWarning;
        if (errorCount != 0)
        {
            errorAndWarning += std::to_string(errorCount) + " error" + (errorCount > 0 ? "s" : "") + " generated";
        }
        if (warningCount != 0)
        {
            if (errorCount != 0)
            {
                errorAndWarning += " and ";
            }
            errorAndWarning += std::to_string(warningCount) + " warning" + (errorCount > 0 ? "s" : "") + " generated";
        }
        if (!errorAndWarning.empty())
        {
            jcutils::printWarningBy(errorAndWarning);
        }

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
                hasError |= !compile();
                break;
            }
            case JavacAction::Inspect:
            {
                hasError |= !inspect();
                break;
            }
            case JavacAction::Lexer:
            {
                hasError |= !lexExperimental();
                break;
            }
            case JavacAction::Parser:
            {
                hasError |= !parseExperimental();
                break;
            }
            default:
            {
                jcutils::printErrorCustom("Unknown compiler starting action, this should not happen");
                return false;
            }
        }

        if (hasError)
        {
            printTerminate();
            return false;
        }

        bool post = postProcess();
        if (!post)
        {
            printTerminate();
            return false;
        }

        endTime = jcutils::getMSTime();

        jcutils::printInfoBy("Finished compiling, time took: " + std::to_string((endTime - startTime)) + "ms");

        return true;
    }
} // javac
