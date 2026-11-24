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

    using Color = PrintColor;
    inline Color DEFAULT_COLOR = Color::Default;
    inline Color OWNER_COLOR = Color::Default;
    inline Color MESSAGE_COLOR = Color::BrightWhite;
    inline Color PREFIX_COLOR = Color::Default;
    inline Color INFO_COLOR = Color::BrightCyan;
    inline Color DEBUG_COLOR = Color::BrightYellow;
    inline Color SUCCESS_COLOR = Color::BrightGreen;
    inline Color WARNING_COLOR = Color::Magenta;
    inline Color ERROR_COLOR = Color::BrightRed;

    std::string colorString(std::string_view str);
    void print(std::string_view str = {});
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

    void println(std::string_view str = {});
    void printlnBy(std::string_view str = {});
    void printlnCustom(std::string_view str = {}, std::string_view owner = {});
    void printlnPrefixBy(std::string_view str = {});
    void printlnPrefixBy(std::string_view prefix, std::string_view str);
    void printlnPrefixCustom(std::string_view prefix, std::string_view str, std::string_view owner = {});
    void printlnInfoBy(std::string_view str = {});
    void printlnInfoCustom(std::string_view str = {}, std::string_view owner = {});
    void printlnDebugBy(std::string_view str = {});
    void printlnDebugCustom(std::string_view str = {}, std::string_view owner = {});
    void printlnSuccessBy(std::string_view str = {});
    void printlnSuccessCustom(std::string_view str = {}, std::string_view owner = {});
    void printlnWarningBy(std::string_view str = {});
    void printlnWarningCustom(std::string_view str = {}, std::string_view owner = {});
    void printlnErrorBy(std::string_view str = {});
    void printlnErrorCustom(std::string_view str = {}, std::string_view owner = {});

}

#endif //JAVACOMPILER_PRINTUTIL_H
