//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_FILEUTIL_H
#define JAVACOMPILER_FILEUTIL_H

#include <string>

#include "main/core/DataDef.h"

namespace jcutils
{
    [[nodiscard]]
    Path joinAbsolutePath(const Path& directory, const Path& fileName);

    [[nodiscard]]
    Path joinPath(const Path& directory, const Path& fileName);

    [[nodiscard]]
    Path getRelativePath(const Path& filePath, const Path& directoryPath);

    [[nodiscard]]
    Path changeExtension(Path path, const Path& extension);

    [[nodiscard]]
    std::string getExtension(const Path& path);
} // jcutils

#endif //JAVACOMPILER_FILEUTIL_H
