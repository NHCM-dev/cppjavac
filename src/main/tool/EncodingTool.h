//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_ENCODINGTOOL_H
#define JAVACOMPILER_ENCODINGTOOL_H
#include <string>
#include <string_view>

#include "main/core/def/TextEncoding.h"

class EncodingTool
{
public:
    // Throws std::invalid_argument for malformed UTF-8.
    static std::u16string utf8ToUtf16(const std::string& string);
    static std::string utf16ToUtf8(std::u16string_view text);
    static std::u16string decodeToUtf16(const std::string& bytes, TextEncoding originalEncoding);
};


#endif //JAVACOMPILER_ENCODINGTOOL_H
