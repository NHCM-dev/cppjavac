//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_FILEIOTOOL_H
#define JAVACOMPILER_FILEIOTOOL_H
#include <string>

#include "main/core/CoreDef.h"


class FileIOTool
{
public:
    // Reads UTF-8 bytes verbatim. Throws std::ios_base::failure on I/O errors.
    static std::string readUtf8(Path path);
};


#endif //JAVACOMPILER_FILEIOTOOL_H
