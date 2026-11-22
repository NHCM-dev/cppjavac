//
// Created by NHCM on 2026/11/22.
//

#include "PrintUtil.h"

#include <iostream>

namespace jcutils
{
    void print(std::string_view str, PrintColor color)
    {
        std::cout << "\033["
                  << static_cast<int>(color)
                  << "m"
                  << str
                  << "\033[0m";
    }
    void println(std::string_view str, PrintColor color)
    {
        print(str, color);
        std::cout << '\n';
    }
}
