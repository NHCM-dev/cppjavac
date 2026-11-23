//
// Created by NHCM on 2026/11/22.
//

#include "FileIoTool.h"

#include <fstream>

std::string FileIOTool::readUtf8(Path path)
{
    const auto fileName = path.u8string();
    const std::string displayPath(fileName.begin(), fileName.end());
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        throw std::ios_base::failure("Cannot open file: " + displayPath);
    }

    std::string result;
    char buffer[8192];
    while (input.read(buffer, sizeof(buffer)))
    {
        result.append(buffer, sizeof(buffer));
    }
    if (input.bad() || !input.eof())
    {
        throw std::ios_base::failure("Cannot read file: " + displayPath);
    }
    result.append(buffer, static_cast<std::size_t>(input.gcount()));
    return result;
}
