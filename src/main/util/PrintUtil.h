//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_PRINTUTIL_H
#define JAVACOMPILER_PRINTUTIL_H

#include <cstdint>
#include <iosfwd>
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

    using Color = PrintColor;
    inline Color DEFAULT_COLOR = Color::Default;
    inline Color OWNER_COLOR = Color::Default;
    inline Color MESSAGE_COLOR = Color::BrightWhite;
    inline Color PREFIX_COLOR = Color::Default;
    inline Color INFO_COLOR = Color::BrightCyan;
    inline Color DEBUG_COLOR = Color::BrightYellow;
    inline Color SUCCESS_COLOR = Color::BrightGreen;
    inline Color WARNING_COLOR = Color::Magenta;
    inline Color ERROR_COLOR = Color::Red;
    inline Color NOTE_COLOR = Color::BrightBlack;
    inline Color REMARK_COLOR = Color::Blue;
    inline Color FATAL_ERROR_COLOR = Color::Red;

    inline Color DIAGNOSTIC_PATH_COLOR = Color::BrightGreen;
    inline Color DIAGNOSTIC_SITE_COLOR = Color::BrightCyan;

    void printDiagnosticHeader(std::string_view message, std::string_view owner,
        std::string_view path, std::string_view location, std::string_view severity, Color severityColor);

    std::string colorString(std::string_view str);
    void print(std::string_view str = {});
    void print(std::ostream& output, std::string_view str);
    void printBy(std::string_view str = {});
    void printCustom(std::string_view str = {}, std::string_view owner = {});
    void printPrefixBy(std::string_view str = {});
    void printPrefixBy(std::string_view prefix, std::string_view str);
    void printPrefixCustom(std::string_view prefix, std::string_view str, std::string_view owner = {});
    void printInfoBy(std::string_view str = {});
    void printInfoCustom(std::string_view str = {}, std::string_view owner = {});
    void printDebugBy(std::string_view str = {});
    void printDebugCustom(std::string_view str = {}, std::string_view owner = {});
    void printSuccessBy(std::string_view str = {});
    void printSuccessCustom(std::string_view str = {}, std::string_view owner = {});
    void printWarningBy(std::string_view str = {});
    void printWarningCustom(std::string_view str = {}, std::string_view owner = {});
    void printErrorBy(std::string_view str = {});
    void printErrorCustom(std::string_view str = {}, std::string_view owner = {});
    void printNoteBy(std::string_view str = {});
    void printNoteCustom(std::string_view str = {}, std::string_view owner = {});
    void printRemarkBy(std::string_view str = {});
    void printRemarkCustom(std::string_view str = {}, std::string_view owner = {});
    void printFatalErrorBy(std::string_view str = {});
    void printFatalErrorCustom(std::string_view str = {}, std::string_view owner = {});

}

#endif //JAVACOMPILER_PRINTUTIL_H
