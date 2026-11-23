//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_MATHUTIL_H
#define JAVACOMPILER_MATHUTIL_H

#include <cstdint>
#include <string_view>

namespace jcutils
{
    /**
     * @throws std::invalid_argument if the input string is not an integer
     * @param str string to be parsed at integer
     * @return parsed integer
     */
    int parseInt(std::string_view str);

    // Check Java literal syntax without converting the value or checking its range.
    [[nodiscard]] bool isInteger(std::string_view str);
    [[nodiscard]] bool isInteger(std::u16string_view str);
    [[nodiscard]] bool isFloat(std::string_view str);
    [[nodiscard]] bool isFloat(std::u16string_view str);

    [[nodiscard]] uint64_t getMSTime();

} // jcutils

#endif //JAVACOMPILER_MATHUTIL_H
