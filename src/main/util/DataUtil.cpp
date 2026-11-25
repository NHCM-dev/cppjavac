//
// Created by NHCM on 2026/11/22.
//

#include "DataUtil.h"

#include <algorithm>
#include <cctype>

namespace jcutils
{
    std::vector<std::string_view> splitLines(std::string_view str)
    {
        std::vector<std::string_view> lines;
        std::size_t start = 0;

        while (true)
        {
            std::size_t end = str.find('\n', start);

            auto line = str.substr(
                start,
                end == std::string_view::npos
                    ? str.size() - start
                    : end - start
            );

            if (!line.empty() && line.back() == '\r')
                line.remove_suffix(1);

            lines.push_back(line);

            if (end == std::string_view::npos)
                break;

            start = end + 1;
        }

        return lines;
    }

    std::string toLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });

        return str;
    }

    bool hasSpace(std::string_view str)
    {
        return std::ranges::any_of(str,[](char16_t c)
       {
           return c == u' ' ||c == u'\t' ||c == u'\n' ||c == u'\r' ||c == u'\f';
       });
    }

    bool hasSpace(std::u16string_view str)
    {
        return std::ranges::any_of(str,[](char16_t c)
       {
           return c == u' ' ||c == u'\t' ||c == u'\n' ||c == u'\r' ||c == u'\f';
       });
    }
}
