//
// Created by NHCM on 2026/11/22.
//

#include "PrintUtil.h"

#include "main/JavaCompiler.h"

#include <iostream>

#ifdef _WIN32
#include "main/tool/EncodingTool.h"

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
        const HANDLE console = GetStdHandle(&output == &std::cerr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
        DWORD mode;
        if (!str.empty() && GetConsoleMode(console, &mode))
        {
            try
            {
                const auto wideText = EncodingTool::utf8ToUtf16(formatColor(str, color));
                output.flush();
                DWORD written;
                if (WriteConsoleW(console, wideText.data(), static_cast<DWORD>(wideText.size()), &written, nullptr))
                {
                    return;
                }
            }
            catch (const std::invalid_argument&)
            {
                // Preserve raw bytes if a system diagnostic is not UTF-8.
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
        std::string_view owner, std::string_view prefix, Color prefixColor)
    {
        if (!owner.empty())
        {
            writeColored(output, owner, OWNER_COLOR);
            output << ": ";
        }
        if (!prefix.empty())
        {
            writeColored(output, prefix, prefixColor);
            output << ": ";
        }
        writeColored(output, str, MESSAGE_COLOR);
    }

    std::string colorString(std::string_view str)
    {
        return formatColor(str, DEFAULT_COLOR);
    }

    void print(std::string_view str)
    {
        writeColored(std::cout, str, DEFAULT_COLOR);
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

    void println(std::string_view str)
    {
        writeColored(std::cout, str, DEFAULT_COLOR);
        std::cout.put('\n');
    }

    void printlnBy(std::string_view str)
    {
        printlnCustom(str, javac::program);
    }

    void printlnCustom(std::string_view str, std::string_view owner)
    {
        printlnPrefixCustom({}, str, owner);
    }

    void printlnPrefixBy(std::string_view str)
    {
        printlnBy(str);
    }

    void printlnPrefixBy(std::string_view prefix, std::string_view str)
    {
        printlnPrefixCustom(prefix, str, javac::program);
    }

    void printlnPrefixCustom(std::string_view prefix, std::string_view str, std::string_view owner)
    {
        writeMessage(std::cout, str, owner, prefix, PREFIX_COLOR);
        std::cout.put('\n');
    }

    void printlnInfoBy(std::string_view str)
    {
        printlnInfoCustom(str, javac::program);
    }

    void printlnInfoCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cout, str, owner, "info", INFO_COLOR);
        std::cout.put('\n');
    }

    void printlnDebugBy(std::string_view str)
    {
        printlnDebugCustom(str, javac::program);
    }

    void printlnDebugCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cout, str, owner, "debug", DEBUG_COLOR);
        std::cout.put('\n');
    }

    void printlnSuccessBy(std::string_view str)
    {
        printlnSuccessCustom(str, javac::program);
    }

    void printlnSuccessCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cout, str, owner, "success", SUCCESS_COLOR);
        std::cout.put('\n');
    }

    void printlnWarningBy(std::string_view str)
    {
        printlnWarningCustom(str, javac::program);
    }

    void printlnWarningCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cerr, str, owner, "warning", WARNING_COLOR);
        std::cerr.put('\n');
    }

    void printlnErrorBy(std::string_view str)
    {
        printlnErrorCustom(str, javac::program);
    }

    void printlnErrorCustom(std::string_view str, std::string_view owner)
    {
        writeMessage(std::cerr, str, owner, "error", ERROR_COLOR);
        std::cerr.put('\n');
    }
}
