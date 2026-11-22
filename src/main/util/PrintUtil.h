//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_PRINTUTIL_H
#define JAVACOMPILER_PRINTUTIL_H

#include <cstdint>
#include <iostream>

namespace jcutils
{
    enum class PrintColor : uint16_t
    {
        Default = 39,
        Black = 30,
        Red = 31,
        Green = 32,
        Yellow = 33,
        Blue = 34,
        Magenta = 35,
        Cyan = 36,
        White = 37
    };

    void print(std::string_view str = "", PrintColor color = PrintColor::Default);
    void println(std::string_view str = "", PrintColor color = PrintColor::Default);


}

#endif //JAVACOMPILER_PRINTUTIL_H
