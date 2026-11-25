//
// Created by NHCM on 2026/11/25.
//

#ifndef CPPJAVAC_PARSERUTIL_H
#define CPPJAVAC_PARSERUTIL_H
#include <string_view>
#include <stdexcept>

#include "main/core/def/JavaDef.h"

namespace jcutils
{
    /**
     * Parse u16 string as int64 which is long in java def
     * @throw std::invalid_argument if entered string is invalid
     * @throw std::out_of_range if entered string is out of int64_t range
     * @param str UTF-16 decimal integer with an optional sign and l/L suffix
     * @return parsed int64_t result
     */
    javac::jlong int64FromString(std::u16string_view str);
    // Return true if the value is outside int32_t or the parsed spelling has an l/L suffix.
    bool int64IsLong(javac::jlong val, std::u16string_view spelling = {});

    /**
     * Parse a decimal or hexadecimal floating-point number as a double.
     * @throw std::invalid_argument if str has invalid syntax
     * @throw std::out_of_range if the value overflows or underflows to zero
     * @param str UTF-16 number with an optional sign and Java floating-point suffix
     * @return parsed double value
     */
    javac::jdouble doubleFromString(std::u16string_view str);

    // Return true if the spelling is a Java floating-point literal with an f/F suffix.
    bool is32Float(std::u16string_view str);

    javac::jboolean boolFromString(std::u16string_view input);
} // jcutils

#endif //CPPJAVAC_PARSERUTIL_H
