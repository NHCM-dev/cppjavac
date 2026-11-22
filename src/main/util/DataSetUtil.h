//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_DATASETUTIL_H
#define JAVACOMPILER_DATASETUTIL_H

#include "../core/CoreDef.h"

#include <algorithm>

namespace jcutils
{
    template <typename T>
    bool contains(const std::vector<T>& vector, const T& element)
    {
        return std::ranges::contains(vector, element);
    }
}

#endif //JAVACOMPILER_DATASETUTIL_H
