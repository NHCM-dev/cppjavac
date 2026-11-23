//
// Created by NHCM on 2026/11/22.
//

#include "MathUtil.h"

#include <charconv>
#include <chrono>

namespace jcutils
{
    static bool isDigitForBase(char32_t ch, int base)
    {
        if (ch >= '0' && ch <= '9')
        {
            return ch - '0' < static_cast<char32_t>(base);
        }
        return base == 16 && ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'));
    }

    template <typename Char>
    static bool consumeDigits(std::basic_string_view<Char>& str, int base)
    {
        if (str.empty() || !isDigitForBase(str.front(), base))
        {
            return false;
        }

        std::size_t end = 1;
        while (end < str.size())
        {
            std::size_t next = end;
            while (next < str.size() && str[next] == '_')
            {
                ++next;
            }
            if (next == str.size() || !isDigitForBase(str[next], base))
            {
                break;
            }
            end = next + 1;
        }
        str.remove_prefix(end);
        return true;
    }

    template <typename Char>
    static bool hasRadixPrefix(std::basic_string_view<Char> str, char lower, char upper)
    {
        return str.size() >= 2 && str[0] == '0' && (str[1] == lower || str[1] == upper);
    }

    template <typename Char>
    static bool isIntegerLiteral(std::basic_string_view<Char> str)
    {
        if (!str.empty() && (str.back() == 'l' || str.back() == 'L'))
        {
            str.remove_suffix(1);
        }
        if (str.empty())
        {
            return false;
        }

        int base = str.front() == '0' ? 8 : 10;
        if (hasRadixPrefix(str, 'x', 'X'))
        {
            base = 16;
            str.remove_prefix(2);
        }
        else if (hasRadixPrefix(str, 'b', 'B'))
        {
            base = 2;
            str.remove_prefix(2);
        }
        return consumeDigits(str, base) && str.empty();
    }

    template <typename Char>
    static bool isFloatingPointLiteral(std::basic_string_view<Char> str)
    {
        const bool hex = hasRadixPrefix(str, 'x', 'X');
        const int base = hex ? 16 : 10;
        if (hex)
        {
            str.remove_prefix(2);
        }

        const bool wholeDigits = consumeDigits(str, base);
        const bool hasDot = !str.empty() && str.front() == '.';
        bool fractionalDigits = false;
        if (hasDot)
        {
            str.remove_prefix(1);
            fractionalDigits = consumeDigits(str, base);
        }
        if (!wholeDigits && !fractionalDigits)
        {
            return false;
        }

        const bool hasExponent = !str.empty()
            && (hex ? (str.front() == 'p' || str.front() == 'P')
                    : (str.front() == 'e' || str.front() == 'E'));
        if (hasExponent)
        {
            str.remove_prefix(1);
            if (!str.empty() && (str.front() == '+' || str.front() == '-'))
            {
                str.remove_prefix(1);
            }
            if (!consumeDigits(str, 10))
            {
                return false;
            }
        }
        if (hex && !hasExponent)
        {
            return false;
        }

        const bool hasSuffix = !str.empty()
            && (str.front() == 'f' || str.front() == 'F' || str.front() == 'd' || str.front() == 'D');
        if (hasSuffix)
        {
            str.remove_prefix(1);
        }
        return str.empty() && (hasDot || hasExponent || hasSuffix);
    }

    bool isInteger(std::string_view str)
    {
        return isIntegerLiteral(str);
    }

    bool isInteger(std::u16string_view str)
    {
        return isIntegerLiteral(str);
    }

    bool isFloat(std::string_view str)
    {
        return isFloatingPointLiteral(str);
    }

    bool isFloat(std::u16string_view str)
    {
        return isFloatingPointLiteral(str);
    }

    int parseInt(std::string_view str)
    {
        int value;

        auto [ptr, ec] = std::from_chars(
            str.data(),
            str.data() + str.size(),
            value
        );

        if (ec != std::errc{} ||
            ptr != str.data() + str.size())
        {
            throw std::invalid_argument("Invalid number");
        }

        return value;
    }

    uint64_t getMSTime()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
            ).count();
    }
} // jcutils
