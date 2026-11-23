//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_FILEUTIL_H
#define JAVACOMPILER_FILEUTIL_H

#include "main/core/CoreDef.h"

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
} // jcutils

#endif //JAVACOMPILER_FILEUTIL_H
