//
// Created by NHCM on 2026/11/22.
//

#include "TextEncoding.h"

std::optional<TextEncoding> parseEncoding(std::string_view encoding)
{
    const std::string in = jcutils::toLower(std::string(encoding));

    if (in == "ascii")
        return TextEncoding::ASCII;

    if (in == "utf8" || in == "utf-8")
        return TextEncoding::UTF8;

    if (in == "utf16le" || in == "utf-16le")
        return TextEncoding::UTF16LE;

    if (in == "utf16be" || in == "utf-16be")
        return TextEncoding::UTF16BE;

    if (in == "utf32le" || in == "utf-32le")
        return TextEncoding::UTF32LE;

    if (in == "utf32be" || in == "utf-32be")
        return TextEncoding::UTF32BE;

    if (in == "gbk")
        return TextEncoding::GBK;

    if (in == "gb18030" || in == "gb-18030")
        return TextEncoding::GB18030;

    if (in == "iso8859-1" || in == "iso-8859-1" ||
        in == "latin1" || in == "latin-1")
        return TextEncoding::ISO8859_1;

    if (in == "windows1252" || in == "windows-1252" ||
        in == "cp1252")
        return TextEncoding::Windows1252;

    return std::nullopt;
}