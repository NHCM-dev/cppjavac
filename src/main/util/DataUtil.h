//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_DATASETUTIL_H
#define JAVACOMPILER_DATASETUTIL_H

#include <algorithm>
#include <map>
#include <ranges>
#include <set>
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

    /**
     * Test if an element is already inserted or automatically insert into a std::set
     * @param set target set
     * @param value elements to insert
     * @return Returns true if inserted successfully, false if the set already contains the elements
     */
    template<typename T>
    bool containsOrInsert(std::set<T>& set, const T& value)
    {
        return set.insert(value).second;
    }

    std::string toLower(std::string str);
}

#endif //JAVACOMPILER_DATASETUTIL_H
