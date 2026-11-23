//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_TEXTENCODING_H
#define JAVACOMPILER_TEXTENCODING_H

#include <cstdint>

#include "main/util/DataUtil.h"

enum class TextEncoding : uint8_t
{
    ASCII = 0,
    UTF8 = 1,
    UTF16LE = 2,
    UTF16BE = 3,
    GBK = 6,
};

std::optional<TextEncoding> parseEncoding(std::string_view encoding);

#endif //JAVACOMPILER_TEXTENCODING_H
