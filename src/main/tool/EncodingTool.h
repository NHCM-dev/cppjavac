//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_ENCODINGTOOL_H
#define JAVACOMPILER_ENCODINGTOOL_H
#include <string>


class EncodingTool
{
public:
    // Throws std::invalid_argument for malformed UTF-8.
    static std::u16string utf8ToUtf16(const std::string& string);
};


#endif //JAVACOMPILER_ENCODINGTOOL_H
