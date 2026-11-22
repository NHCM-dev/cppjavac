//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_MATHUTIL_H
#define JAVACOMPILER_MATHUTIL_H

#include <string_view>
#include <stdexcept>

namespace jcutils
{
    /**
     * @throws std::invalid_argument if the input string is not an integer
     * @param str string to be parsed at integer
     * @return parsed integer
     */
    int parseInt(std::string_view str);

} // jcutils

#endif //JAVACOMPILER_MATHUTIL_H
