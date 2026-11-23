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

    if (in == "gbk")
        return TextEncoding::GBK;

    return std::nullopt;
}