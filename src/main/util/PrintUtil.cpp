//
// Created by NHCM on 2026/11/22.
//

#include "PrintUtil.h"

#include <iostream>

namespace jcutils
{
    void print(std::string str)
    {
        std::cout << str;
    }

    void print(char* str)
    {
        std::cout << str;
    }

    void println(std::string str)
    {
        std::cout << str << std::endl;
    }

    void println(char* str)
    {
        std::cout << str << std::endl;
    }
}
