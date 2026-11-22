//
// Created by NHCM on 2026/11/22.
//

#include "DataUtil.h"

#include <algorithm>
#include <cctype>

namespace jcutils
{
    std::string toLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });

        return str;
    }
}
