//
// Created by NHCM on 2026/11/24.
//

#include "ExceptionUtil.h"

#include <algorithm>
#include <iostream>

#include "main/tool/EncodingTool.h"
#include "PrintUtil.h"

namespace jcutils
{
    namespace
    {
        SourceLocation locate(std::u16string_view source, std::size_t offset)
        {
            offset = std::min(offset, source.size());
            if (offset > 0 && source.substr(offset - 1, 2) == u"\r\n")
            {
                --offset;
            }
            std::size_t line = 1;
            std::size_t start = 0;
            for (auto end = source.find_first_of(u"\r\n"); end < offset;
                end = source.find_first_of(u"\r\n", start))
            {
                start = end + (source.substr(end, 2) == u"\r\n" ? 2 : 1);
                ++line;
            }
            return {offset, line, offset - start + 1};
        }

        struct DisplayLine
        {
            std::u16string text;
            std::vector<std::size_t> columns{0};
        };

        DisplayLine expandLine(std::u16string_view line)
        {
            constexpr std::size_t tabWidth = 8;
            DisplayLine display;
            std::size_t column = 0;
            for (std::size_t i = 0; i < line.size(); ++i)
            {
                const auto ch = line[i];
                if (ch == u'\t')
                {
                    const auto spaces = tabWidth - column % tabWidth;
                    display.text.append(spaces, u' ');
                    column += spaces;
                }
                else
                {
                    display.text += ch;
                    if (ch >= 0xD800 && ch <= 0xDBFF && i + 1 < line.size()
                        && line[i + 1] >= 0xDC00 && line[i + 1] <= 0xDFFF)
                    {
                        display.columns.push_back(column);
                        display.text += line[++i];
                    }
                    ++column;
                }
                display.columns.push_back(column);
            }
            return display;
        }

        void printExcerpt(const DiagnosticSite& site, std::u16string_view source)
        {
            const auto begin = site.begin;
            auto end = site.end.value_or(begin);
            if (end.line < begin.line || (end.line == begin.line && end.column < begin.column))
            {
                end = begin;
            }
            const auto lastLine = end.line;
            const auto numberWidth = std::to_string(lastLine).size() + 3;
            const auto markerPrefix = std::string(numberWidth, ' ') + " | ";

            for (auto lineNumber = begin.line; ; ++lineNumber)
            {
                if (lineNumber != begin.line && lineNumber == end.line && end.column <= 1)
                {
                    break;
                }

                const auto lineEnd = source.find_first_of(u"\r\n");
                const auto display = expandLine(source.substr(0, lineEnd));
                const auto number = std::to_string(lineNumber);
                print(std::cerr, std::string(numberWidth - number.size(), ' ') + number + " | " + EncodingTool::utf16ToUtf8(display.text));

                const auto displayColumn = [&](std::size_t column)
                {
                    return display.columns[std::min(column > 0 ? column - 1 : 0, display.columns.size() - 1)];
                };
                const auto start = lineNumber == begin.line ? displayColumn(begin.column) : 0;
                const auto finish = lineNumber == end.line ? displayColumn(end.column) : display.columns.back();
                const auto length = finish > start ? finish - start : 1;
                const char marker = lineNumber == begin.line ? '^' : '~';
                print(std::cerr, markerPrefix + std::string(start, ' ') + marker + std::string(length - 1, '~'));

                if (lineNumber == lastLine || lineEnd == std::string_view::npos)
                {
                    break;
                }
                source.remove_prefix(lineEnd + (source.substr(lineEnd, 2) == u"\r\n" ? 2 : 1));
            }
        }
    }

    void printDiagnostic(
        DiagnosticSeverity severity,
        std::optional<Path> path,
        std::string_view message,
        std::optional<DiagnosticSite> site,
        std::optional<std::u16string_view> sourceFile,
        std::string_view owner)
    {
        if (site && sourceFile)
        {
            site->begin = locate(*sourceFile, site->begin.offsetInFile);
            if (site->end)
            {
                site->end = locate(*sourceFile, site->end->offsetInFile);
            }
        }

        const auto filePath = path ? path->string() : std::string{};
        const auto location = site ? sourceLocationToString(site->begin) : std::string{};

        switch (severity)
        {
            case DiagnosticSeverity::Error:
                printDiagnosticHeader(message, owner, filePath, location, "error", ERROR_COLOR);
                break;
            case DiagnosticSeverity::Warning:
                printDiagnosticHeader(message, owner, filePath, location, "warning", WARNING_COLOR);
                break;
            case DiagnosticSeverity::Note:
                printDiagnosticHeader(message, owner, filePath, location, "note", NOTE_COLOR);
                break;
            case DiagnosticSeverity::Remark:
                printDiagnosticHeader(message, owner, filePath, location, "remark", REMARK_COLOR);
                break;
            case DiagnosticSeverity::FatalError:
                printDiagnosticHeader(message, owner, filePath, location, "fatal error", FATAL_ERROR_COLOR);
                break;
        }

        if (site && sourceFile)
        {
            const auto lineStart = site->begin.offsetInFile - (site->begin.column - 1);
            printExcerpt(*site, sourceFile->substr(lineStart));
        }
    }

} // jcutils
