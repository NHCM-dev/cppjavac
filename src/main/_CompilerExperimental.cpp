//
// Created by NHCM on 2026/11/26.
//

#include "Compiler.h"
#include "lexer/Lexer.h"
#include "tool/EncodingTool.h"
#include "tool/FileIoTool.h"
#include "tool/compiler/ASTRenderTool.h"
#include "tool/compiler/TokenStreamRenderTool.h"
#include "util/ExceptionUtil.h"
#include "util/PrintUtil.h"

namespace javac
{
    bool Compiler::lexExperimental()
    {
        jcutils::printWarningCustom("This lex feature is currently experimental", LEXER);
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
                tokenStream = lexer.lex(true, true);
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
            const auto output = TokenStreamRenderTool::renderListing(sourceDisplay, tokenStream);
            const auto* bytes = reinterpret_cast<const std::byte*>(output.data());
            const Path destination = getWritePath(file, "javal");
            if (!putFileToWrite(destination, ByteArray(bytes, bytes + output.size() * sizeof(char16_t))))
            {
                noticeError("Duplicate lexer output path: " + destination.string(), LEXER);
                hasError = true;
            }
        }

        return !hasError;
    }

    bool Compiler::reportParsingErrorExperimental(const Path& file, std::u16string_view source,
        const ParserResult& parserResult)
    {
        for (const auto& error : parserResult.exceptions)
        {
            std::string message = error.what();
            if (message.empty())
            {
                switch (error.exception)
                {
                    case ParserExceptionType::ExpectSymbol: message = "Expected a symbol"; break;
                    case ParserExceptionType::ExpectKeyword: message = "Expected a keyword"; break;
                    case ParserExceptionType::ExpectIdentifier: message = "Expected an identifier"; break;
                    case ParserExceptionType::InvalidClassMember: message = "Invalid class member declaration"; break;
                    case ParserExceptionType::ConstructorNameMismatch: message = "Constructor name must match its class"; break;
                    case ParserExceptionType::AbstractMethodInNonAbstractClass: message = "Abstract method requires an abstract class"; break;
                    case ParserExceptionType::UnsupportedFieldInitializer: message = "Field initializer parsing is not implemented"; break;
                    case ParserExceptionType::UnsupportedMemberTypeDeclaration: message = "Parsing this nested type is not implemented"; break;
                    case ParserExceptionType::UnsupportedElementExpression: message = "Annotation expression parsing is not implemented"; break;
                    default: message = "Parsing error (code " + std::to_string(static_cast<unsigned>(error.exception)) + ')';
                }
            }
            jcutils::printDiagnostic(jcutils::DiagnosticSeverity::Error, file, message, error.range, source);
            compileRtData->errors.push_back(std::move(message));
        }
        return !parserResult.exceptions.empty();
    }

    bool Compiler::parseExperimental()
    {
        jcutils::printWarningCustom("This parse feature is currently experimental", PARSER);
        bool hasError = !lex();
        hasError |= !parse();
        for (const auto& [file, result] : compileRtData->parserResults)
        {
            if (!result.exceptions.empty())
            {
                continue;
            }
            const std::string output = ASTRenderTool::render(file, result.compilationUnit);
            const auto* bytes = reinterpret_cast<const std::byte*>(output.data());
            const Path destination = getWritePath(file, "ast");
            if (!putFileToWrite(destination, ByteArray(bytes, bytes + output.size())))
            {
                noticeError("Duplicate AST output path: " + destination.string(), PARSER);
                hasError = true;
            }
        }
        return !hasError;
    }
}
