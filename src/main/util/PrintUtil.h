//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_PRINTUTIL_H
#define JAVACOMPILER_PRINTUTIL_H

#include <cstdint>
#include <string>
#include <string_view>

namespace jcutils
{
    enum class PrintColor : uint16_t
    {
        None = 0,
        Default = 39,
        Black = 30,
        Red = 31,
        Green = 32,
        Yellow = 33,
        Blue = 34,
        Magenta = 35,
        Cyan = 36,
        White = 37,

        BrightBlack = 90,
        BrightRed = 91,
        BrightGreen = 92,
        BrightYellow = 93,
        BrightBlue = 94,
        BrightMagenta = 95,
        BrightCyan = 96,
        BrightWhite = 97
    };

    std::string colorString(std::string_view str, PrintColor color = PrintColor::Default);

    void print(std::string_view str = {}, PrintColor color = PrintColor::Default);
    void println(std::string_view str = {}, PrintColor color = PrintColor::Default);

    void printBy(std::string_view str = {},
        PrintColor ownerColor = PrintColor::Default, PrintColor strColor = PrintColor::Default);
    // This overload applies prefixColor to the separator after the owner.
    void printPrefixBy(std::string_view str = {},
        PrintColor prefixColor = PrintColor::Default, PrintColor ownerColor = PrintColor::Default,
        PrintColor strColor = PrintColor::Default);
    void printPrefixBy(std::string_view prefix, std::string_view str,
        PrintColor prefixColor = PrintColor::Default, PrintColor ownerColor = PrintColor::Default,
        PrintColor strColor = PrintColor::Default);

    void printlnBy(std::string_view str = {},
        PrintColor ownerColor = PrintColor::Default, PrintColor strColor = PrintColor::Default);
    void printlnPrefixBy(std::string_view str = {},
        PrintColor prefixColor = PrintColor::Default, PrintColor ownerColor = PrintColor::Default,
        PrintColor strColor = PrintColor::Default);
    void printlnPrefixBy(std::string_view prefix, std::string_view str,
        PrintColor prefixColor = PrintColor::Default, PrintColor ownerColor = PrintColor::Default,
        PrintColor strColor = PrintColor::Default);

    // Diagnostics go to stderr.
    void printErrorBy(std::string_view str = {},
        PrintColor errorColor = PrintColor::BrightRed, PrintColor ownerColor = PrintColor::Default,
        PrintColor strColor = PrintColor::Default);
    void printWarningBy(std::string_view str = {},
        PrintColor warningColor = PrintColor::BrightYellow, PrintColor ownerColor = PrintColor::Default,
        PrintColor strColor = PrintColor::Default);
    void printlnErrorBy(std::string_view str = {},
        PrintColor errorColor = PrintColor::BrightRed, PrintColor ownerColor = PrintColor::Default,
        PrintColor strColor = PrintColor::Default);
    void printlnWarningBy(std::string_view str = {},
        PrintColor warningColor = PrintColor::BrightYellow, PrintColor ownerColor = PrintColor::Default,
        PrintColor strColor = PrintColor::Default);
}

#endif //JAVACOMPILER_PRINTUTIL_H
