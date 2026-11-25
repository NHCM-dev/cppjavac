//
// Created by NHCM on 2026/11/22.
//

#include "PrintUtil.h"

#include "main/cppjavac.h"
#include "main/tool/EncodingTool.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace jcutils
{
    constexpr std::string_view colorStart = "\033[";
    constexpr std::string_view colorReset = "\033[0m";

    static std::string formatColor(std::string_view str, PrintColor color)
    {
        if (str.empty() || color == PrintColor::None)
        {
            return std::string(str);
        }

        const std::string code = std::to_string(static_cast<int>(color));
        std::string result;
        result.reserve(colorStart.size() + code.size() + 1 + str.size() + colorReset.size());
        result.append(colorStart).append(code).append(1, 'm').append(str).append(colorReset);
        return result;
    }

    static void writeColored(std::ostream& output, std::string_view str, PrintColor color)
    {
        #ifdef _WIN32
        if (&output == &std::cout || &output == &std::cerr)
        {
            HANDLE console = GetStdHandle(&output == &std::cerr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
            DWORD mode;
            if (!str.empty() && GetConsoleMode(console, &mode))
            {
                try
                {
                    const auto wideText = EncodingTool::utf8ToUtf16(formatColor(str, color));
                    output.flush();
                    DWORD written;
                    if (WriteConsoleW(console, wideText.data(), wideText.size(), &written, nullptr))
                    {
                        return;
                    }
                }
                catch (const std::invalid_argument&)
                {
                    // Preserve raw bytes if a system diagnostic is not UTF-8.
                }
            }
        }
        #endif
        if (str.empty() || color == PrintColor::None)
        {
            output << str;
        }
        else
        {
            output << colorStart << std::to_string(static_cast<int>(color)) << 'm' << str << colorReset;
        }
    }

    static void writeMessage(std::ostream& output, std::string_view str,
        std::string_view owner, std::string_view prefix, Color prefixColor,
        std::string_view path = {}, std::string_view location = {})
    {
        if (!owner.empty())
        {
            writeColored(output, owner, OWNER_COLOR);
            output << ": ";
        }
        if (!path.empty())
        {
            writeColored(output, path, DIAGNOSTIC_PATH_COLOR);
            output << (location.empty() ? ": " : ":");
        }
        if (!location.empty())
        {
            writeColored(output, location, DIAGNOSTIC_SITE_COLOR);
            output << ": ";
        }
        if (!prefix.empty())
        {
            writeColored(output, prefix, prefixColor);
            output << ": ";
        }
        writeColored(output, str, MESSAGE_COLOR);
        output.put('\n');
    }

    void printDiagnosticHeader(std::string_view message, std::string_view owner,
        std::string_view path, std::string_view location, std::string_view severity, Color severityColor)
    {
        writeMessage(std::cerr, message, owner, severity, severityColor, path, location);
    }

    std::string colorString(std::string_view str)
    {
        return formatColor(str, DEFAULT_COLOR);
    }

    void print(std::string_view str)
    {
        print(std::cout, str);
    }

    void print(std::ostream& output, std::string_view str)
    {
        writeColored(output, str, DEFAULT_COLOR);
        output.put('\n');
    }

    void printBy(std::string_view str)
    {
        printCustom(str, javac::program);
    }

    void printCustom(std::string_view str, std::string_view owner)
    {
        printPrefixCustom({}, str, owner);
    }

    void printPrefixBy(std::string_view str)
    {
        printBy(str);
    }

    void printPrefixBy(std::string_view prefix, std::string_view str)
    {
        printPrefixCustom(prefix, str, javac::program);
    }

    void printPrefixCustom(std::string_view prefix, std::string_view str, std::string_view owner)
    {
        writeMessage(std::cout, str, owner, prefix, PREFIX_COLOR);
    }

    void printInfoBy(std::string_view str)
    {
        printInfoCustom(str, javac::program);
    }

    void printInfoCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cout, str, owner, "info", INFO_COLOR);
    }

    void printDebugBy(std::string_view str)
    {
        printDebugCustom(str, javac::program);
    }

    void printDebugCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cout, str, owner, "debug", DEBUG_COLOR);
    }

    void printSuccessBy(std::string_view str)
    {
        printSuccessCustom(str, javac::program);
    }

    void printSuccessCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cout, str, owner, "success", SUCCESS_COLOR);
    }

    void printWarningBy(std::string_view str)
    {
        printWarningCustom(str, javac::program);
    }

    void printWarningCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cerr, str, owner, "warning", WARNING_COLOR);
    }

    void printErrorBy(std::string_view str)
    {
        printErrorCustom(str, javac::program);
    }

    void printErrorCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cerr, str, owner, "error", ERROR_COLOR);
    }

    void printNoteBy(std::string_view str)
    {
        printNoteCustom(str, javac::program);
    }

    void printNoteCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cerr, str, owner, "note", NOTE_COLOR);
    }

    void printRemarkBy(std::string_view str)
    {
        printRemarkCustom(str, javac::program);
    }

    void printRemarkCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cerr, str, owner, "remark", REMARK_COLOR);
    }

    void printFatalErrorBy(std::string_view str)
    {
        printFatalErrorCustom(str, javac::program);
    }

    void printFatalErrorCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cerr, str, owner, "fatal error", FATAL_ERROR_COLOR);
    }
}
