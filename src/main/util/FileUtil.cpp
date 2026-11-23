//
// Created by NHCM on 2026/11/23.
//

#include "FileUtil.h"

namespace jcutils
{
    Path joinAbsolutePath(const Path& directory, const Path& fileName)
    {
        return fs::absolute(directory / fileName);
    }

    Path joinPath(const Path& directory, const Path& fileName)
    {
        return directory / fileName;
    }

    Path getRelativePath(const Path& filePath, const Path& directoryPath)
    {
        const Path absoluteFile = fs::absolute(filePath).lexically_normal();
        const Path absoluteDirectory = fs::absolute(directoryPath).lexically_normal();
        return absoluteFile.lexically_relative(absoluteDirectory);
    }

    Path changeExtension(Path path, const Path& extension)
    {
        path.replace_extension(extension);
        return path;
    }

    std::string getExtension(const Path& path)
    {
        const auto extension = path.extension().string();
        return std::string(extension.begin(), extension.end());
    }
} // jcutils
