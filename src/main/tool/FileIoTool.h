//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_FILEIOTOOL_H
#define JAVACOMPILER_FILEIOTOOL_H

#include <fstream>
#include <string>

#include "main/core/DataDef.h"

class FileIOTool
{
public:
    // Reads UTF-8 bytes verbatim. Throws std::ios_base::failure on I/O errors.
    static std::string readUtf8(const Path& path);

    /**
     * Write bytes to a file, creating missing parent directories and overwriting existing contents.
     * @param path path of file to write
     * @param bytes bytes of data
     * @return 0 on success, -1 on directory creation or open failure, -2 on write or close failure
     */
    static int writeFile(const Path& path, const std::vector<std::byte>& bytes);
};


#endif //JAVACOMPILER_FILEIOTOOL_H
