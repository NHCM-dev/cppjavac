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
#include <vector>

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

    template<typename T, typename Predicate>
    void removeIf(std::vector<T>& vec, Predicate pred)
    {
        std::vector<T> result;
        result.reserve(vec.size());

        for (auto& element : vec)
        {
            if (!pred(element))
            {
                result.emplace_back(std::move(element));
            }
        }

        vec.swap(result);
    }

    std::vector<std::string_view> splitLines(std::string_view str);

    std::string toLower(std::string str);
}

#endif //JAVACOMPILER_DATASETUTIL_H
