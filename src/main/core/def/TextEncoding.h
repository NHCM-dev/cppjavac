//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_TEXTENCODING_H
#define JAVACOMPILER_TEXTENCODING_H

#include <stdint.h>

enum class TextEncoding : uint8_t
{
    ASCII = 0,
    UTF8 = 1,
    UTF16LE = 2,
    UTF16BE = 3,
    UTF32LE = 4,
    UTF32BE = 5,
    GBK = 6,
    GB18030 = 7,
    ISO8859_1 = 8,
    Windows1252 = 9
};

#endif //JAVACOMPILER_TEXTENCODING_H
