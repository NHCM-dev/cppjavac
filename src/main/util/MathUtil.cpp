//
// Created by NHCM on 2026/11/22.
//

#include "MathUtil.h"

#include <charconv>

namespace jcutils
{
    int parseInt(std::string_view str)
    {
        int value;

        auto [ptr, ec] = std::from_chars(
            str.data(),
            str.data() + str.size(),
            value
        );

        if (ec != std::errc{} ||
            ptr != str.data() + str.size())
        {
            throw std::invalid_argument("Invalid number");
        }

        return value;
    }
} // jcutils