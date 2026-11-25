//
// Created by NHCM on 2026/11/25.
//

#include "ParserUtil.h"

#include <algorithm>
#include <charconv>
#include <limits>

#include "main/tool/EncodingTool.h"
#include "main/util/MathUtil.h"

namespace jcutils
{
    static bool hasLongSuffix(std::u16string_view str)
    {
        return str.ends_with(u'l') || str.ends_with(u'L');
    }

    javac::jlong int64FromString(std::u16string_view str)
    {
        if (hasLongSuffix(str))
        {
            str.remove_suffix(1);
        }
        if (str.starts_with(u'+'))
        {
            str.remove_prefix(1);
            if (str.starts_with(u'-'))
            {
                throw std::invalid_argument("Invalid integer string");
            }
        }

        const auto number = EncodingTool::utf16ToUtf8(str);
        const auto end = number.data() + number.size();
        javac::jlong value;
        const auto result = std::from_chars(number.data(), end, value);
        if (result.ptr != end || result.ec == std::errc::invalid_argument)
        {
            throw std::invalid_argument("Invalid integer string");
        }
        if (result.ec == std::errc::result_out_of_range)
        {
            throw std::out_of_range("Integer overflow");
        }
        return value;
    }

    bool int64IsLong(javac::jlong val, std::u16string_view spelling)
    {
        return hasLongSuffix(spelling)
            || val < std::numeric_limits<std::int32_t>::min()
            || val > std::numeric_limits<std::int32_t>::max();
    }

    javac::jdouble doubleFromString(std::u16string_view str)
    {
        const bool negative = str.starts_with(u'-');
        if (negative || str.starts_with(u'+'))
        {
            str.remove_prefix(1);
        }

        const bool decimalInteger = !str.empty() && std::ranges::all_of(str, [](char16_t ch)
        {
            return ch >= u'0' && ch <= u'9';
        });
        if (!decimalInteger && !isFloat(str))
        {
            throw std::invalid_argument("Invalid floating-point number");
        }

        if (str.ends_with(u'f') || str.ends_with(u'F') || str.ends_with(u'd') || str.ends_with(u'D'))
        {
            str.remove_suffix(1);
        }

        const bool hex = str.starts_with(u"0x") || str.starts_with(u"0X");
        if (hex)
        {
            str.remove_prefix(2);
        }
        auto number = EncodingTool::utf16ToUtf8(str);
        std::erase(number, '_');

        javac::jdouble value;
        const auto end = number.data() + number.size();
        const auto result = std::from_chars(number.data(), end, value,
            hex ? std::chars_format::hex : std::chars_format::general);
        if (result.ec == std::errc::result_out_of_range)
        {
            throw std::out_of_range("Floating-point number out of double range");
        }
        if (result.ec != std::errc{} || result.ptr != end)
        {
            throw std::invalid_argument("Invalid floating-point number");
        }
        return negative ? -value : value;
    }

    bool is32Float(std::u16string_view str)
    {
        return (str.ends_with(u'f') || str.ends_with(u'F')) && isFloat(str);
    }

    javac::jboolean boolFromString(std::u16string_view input)
    {
        return input == u"true";
    }
} // jcutils