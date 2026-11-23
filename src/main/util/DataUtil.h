//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_DATASETUTIL_H
#define JAVACOMPILER_DATASETUTIL_H

#include <algorithm>
#include <map>
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

    template<typename K, typename V, typename Compare, typename Alloc>
    V* getOrNull(std::map<K, V, Compare, Alloc>& map, const K& key)
    {
        auto it = map.find(key);
        return it == map.end() ? nullptr : &it->second;
    }

    std::string toLower(std::string str);
}

#endif //JAVACOMPILER_DATASETUTIL_H
