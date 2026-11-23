//
// Created by NHCM on 2026/11/22.
//

#include "EncodingTool.h"

#include <cstddef>
#include <stdexcept>

std::u16string EncodingTool::utf8ToUtf16(const std::string& string)
{
    std::u16string result;
    result.reserve(string.size());

    for (std::size_t offset = 0; offset < string.size();)
    {
        const std::size_t start = offset;
        const auto invalidUtf8 = [start] {
            return std::invalid_argument("Invalid UTF-8 sequence at byte " + std::to_string(start));
        };
        const auto lead = static_cast<unsigned char>(string[offset++]);
        char32_t codePoint;
        char32_t minimum;
        std::size_t trailingBytes;

        if (lead < 0x80)
        {
            result.push_back(static_cast<char16_t>(lead));
            continue;
        }
        if (lead >= 0xC2 && lead <= 0xDF)
        {
            codePoint = lead & 0x1F;
            minimum = 0x80;
            trailingBytes = 1;
        }
        else if (lead >= 0xE0 && lead <= 0xEF)
        {
            codePoint = lead & 0x0F;
            minimum = 0x800;
            trailingBytes = 2;
        }
        else if (lead >= 0xF0 && lead <= 0xF4)
        {
            codePoint = lead & 0x07;
            minimum = 0x10000;
            trailingBytes = 3;
        }
        else
        {
            throw invalidUtf8();
        }

        if (trailingBytes > string.size() - offset)
        {
            throw invalidUtf8();
        }
        for (std::size_t i = 0; i < trailingBytes; ++i)
        {
            const auto byte = static_cast<unsigned char>(string[offset++]);
            if ((byte & 0xC0) != 0x80)
            {
                throw invalidUtf8();
            }
            codePoint = (codePoint << 6) | (byte & 0x3F);
        }

        if (codePoint < minimum || codePoint > 0x10FFFF
            || (codePoint >= 0xD800 && codePoint <= 0xDFFF))
        {
            throw invalidUtf8();
        }
        if (codePoint <= 0xFFFF)
        {
            result.push_back(static_cast<char16_t>(codePoint));
        }
        else
        {
            codePoint -= 0x10000;
            result.push_back(static_cast<char16_t>(0xD800 + (codePoint >> 10)));
            result.push_back(static_cast<char16_t>(0xDC00 + (codePoint & 0x3FF)));
        }
    }
    return result;
}
