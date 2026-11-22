//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_DATASETUTIL_H
#define JAVACOMPILER_DATASETUTIL_H

#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>

namespace jcutils
{
    template <std::ranges::input_range Container, typename T>
    constexpr bool contains(const Container& container, const T& value)
    {
        return std::ranges::find(container, value) != std::ranges::end(container);
    }

    template <typename... Args>
    constexpr bool matches(std::string_view value, Args... args)
    {
        return ((value == args) || ...);
    }

    std::string toLower(std::string str);
}

#endif //JAVACOMPILER_DATASETUTIL_H
