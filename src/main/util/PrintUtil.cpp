//
// Created by NHCM on 2026/11/22.
//

#include "PrintUtil.h"

#include "main/JavaCompiler.h"

#include <iostream>

namespace jcutils
{
    namespace
    {
        constexpr std::string_view colorStart = "\033[";
        constexpr std::string_view colorReset = "\033[0m";

        void writeColored(std::ostream& output, std::string_view str, PrintColor color)
        {
            if (str.empty() || color == PrintColor::None)
            {
                output << str;
                return;
            }
            output << colorStart << std::to_string(static_cast<int>(color)) << 'm' << str << colorReset;
        }

        void writePrefixed(std::ostream& output, std::string_view prefix,
            std::string_view str, PrintColor prefixColor, PrintColor ownerColor, PrintColor strColor)
        {
            if (!javac::program.empty())
            {
                writeColored(output, javac::program, ownerColor);
                output << ": ";
            }
            if (!prefix.empty())
            {
                writeColored(output, prefix, prefixColor);
                output << ": ";
            }
            writeColored(output, str, strColor);
        }
    }

    std::string colorString(std::string_view str, PrintColor color)
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

    void print(std::string_view str, PrintColor color)
    {
        writeColored(std::cout, str, color);
    }

    void println(std::string_view str, PrintColor color)
    {
        print(str, color);
        std::cout.put('\n');
    }

    void printBy(std::string_view str,
        PrintColor ownerColor, PrintColor strColor)
    {
        writePrefixed(std::cout, {}, str, PrintColor::None, ownerColor, strColor);
    }

    void printPrefixBy(std::string_view str,
        PrintColor prefixColor, PrintColor ownerColor, PrintColor strColor)
    {
        if (!javac::program.empty())
        {
            writeColored(std::cout, javac::program, ownerColor);
            writeColored(std::cout, ": ", prefixColor);
        }
        writeColored(std::cout, str, strColor);
    }

    void printPrefixBy(std::string_view prefix, std::string_view str,
        PrintColor prefixColor, PrintColor ownerColor, PrintColor strColor)
    {
        writePrefixed(std::cout, prefix, str, prefixColor, ownerColor, strColor);
    }

    void printErrorBy(std::string_view str,
        PrintColor errorColor, PrintColor ownerColor, PrintColor strColor)
    {
        writePrefixed(std::cerr, "error", str, errorColor, ownerColor, strColor);
    }

    void printWarningBy(std::string_view str,
        PrintColor warningColor, PrintColor ownerColor, PrintColor strColor)
    {
        writePrefixed(std::cerr, "warning", str, warningColor, ownerColor, strColor);
    }

    void printlnBy(std::string_view str, PrintColor ownerColor, PrintColor strColor)
    {
        printBy(str, ownerColor, strColor);
        std::cout.put('\n');
    }

    void printlnPrefixBy(std::string_view str,
        PrintColor prefixColor, PrintColor ownerColor, PrintColor strColor)
    {
        printPrefixBy(str, prefixColor, ownerColor, strColor);
        std::cout.put('\n');
    }

    void printlnPrefixBy(std::string_view prefix, std::string_view str,
        PrintColor prefixColor, PrintColor ownerColor, PrintColor strColor)
    {
        printPrefixBy(prefix, str, prefixColor, ownerColor, strColor);
        std::cout.put('\n');
    }

    void printlnErrorBy(std::string_view str,
        PrintColor errorColor, PrintColor ownerColor, PrintColor strColor)
    {
        printErrorBy(str, errorColor, ownerColor, strColor);
        std::cerr.put('\n');
    }

    void printlnWarningBy(std::string_view str,
        PrintColor warningColor, PrintColor ownerColor, PrintColor strColor)
    {
        printWarningBy(str, warningColor, ownerColor, strColor);
        std::cerr.put('\n');
    }
}
