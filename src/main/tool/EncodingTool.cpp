//
// Created by NHCM on 2026/11/22.
//

#include "EncodingTool.h"

#include <cstddef>
#include <limits>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#else
#include <iconv.h>
#endif

std::u16string decodeGbk(const std::string& bytes)
{
    if (bytes.empty())
    {
        return {};
    }
    #ifdef _WIN32
    if (bytes.size() > (std::numeric_limits<int>::max)())
    {
        throw std::invalid_argument("GBK source is too large to decode");
    }
    constexpr unsigned codePage = 936;
    const auto length = static_cast<int>(bytes.size());
    const int count = MultiByteToWideChar(codePage, MB_ERR_INVALID_CHARS, bytes.data(), length, nullptr, 0);
    if (count == 0)
    {
        throw std::invalid_argument("Invalid GBK source");
    }
    std::wstring decoded(count, L'\0');
    if (MultiByteToWideChar(codePage, MB_ERR_INVALID_CHARS, bytes.data(), length, decoded.data(), count) == 0)
    {
        throw std::invalid_argument("Invalid GBK source");
    }
    return {decoded.begin(), decoded.end()};
    #else
    std::string utf8(bytes.size() / 2 * 3 + bytes.size() % 2, '\0');
    iconv_t converter = iconv_open("UTF-8", "GBK");
    if (converter == reinterpret_cast<iconv_t>(-1))
    {
        throw std::invalid_argument("GBK decoder is unavailable");
    }
    char* input = const_cast<char*>(bytes.data());
    char* output = utf8.data();
    std::size_t inputLeft = bytes.size();
    std::size_t outputLeft = utf8.size();
    const auto status = iconv(converter, &input, &inputLeft, &output, &outputLeft);
    iconv_close(converter);
    if (status == static_cast<std::size_t>(-1))
    {
        throw std::invalid_argument("Invalid GBK sequence at byte " + std::to_string(bytes.size() - inputLeft));
    }
    utf8.resize(utf8.size() - outputLeft);
    return EncodingTool::utf8ToUtf16(utf8);
    #endif
}

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

std::u16string EncodingTool::decodeToUtf16(const std::string& bytes, TextEncoding originalEncoding)
{
    if (originalEncoding == TextEncoding::UTF8)
    {
        return utf8ToUtf16(bytes);
    }
    if (originalEncoding == TextEncoding::GBK)
    {
        return decodeGbk(bytes);
    }

    std::u16string decoded;
    if (originalEncoding == TextEncoding::ASCII)
    {
        decoded.reserve(bytes.size());
        for (const unsigned char byte : bytes)
        {
            if (byte > 0x7F)
            {
                throw std::invalid_argument("Invalid ASCII byte at offset " + std::to_string(decoded.size()));
            }
            decoded.push_back(byte);
        }
        return decoded;
    }

    const bool littleEndian = originalEncoding == TextEncoding::UTF16LE;
    if (bytes.size() % 2 != 0)
    {
        throw std::invalid_argument("Incomplete UTF-16 code unit at end of source");
    }
    decoded.reserve(bytes.size() / 2);
    for (std::size_t offset = 0; offset < bytes.size(); offset += 2)
    {
        const auto first = static_cast<unsigned char>(bytes[offset]);
        const auto second = static_cast<unsigned char>(bytes[offset + 1]);
        decoded.push_back(static_cast<char16_t>(littleEndian ? (second << 8) | first : (first << 8) | second));
    }
    if (!decoded.empty() && decoded.front() == u'\uFEFF')
    {
        decoded.erase(0, 1);
    }
    bool needLowSurrogate = false;
    for (const char16_t ch : decoded)
    {
        if (needLowSurrogate != (ch >= 0xDC00 && ch <= 0xDFFF))
        {
            throw std::invalid_argument("Unpaired surrogate in UTF-16 source");
        }
        needLowSurrogate = ch >= 0xD800 && ch <= 0xDBFF;
    }
    if (needLowSurrogate)
    {
        throw std::invalid_argument("Unpaired surrogate at end of UTF-16 source");
    }
    return decoded;
}
