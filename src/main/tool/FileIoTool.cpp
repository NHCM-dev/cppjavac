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

int FileIOTool::writeFile(const fs::path& path, const std::vector<std::byte>& bytes)
{
    const Path parent = path.parent_path();
    if (path.has_filename() && !parent.empty())
    {
        std::error_code error;
        fs::create_directories(parent, error);
        if (error)
        {
            return -1;
        }
    }

    std::ofstream file(path, std::ios::binary);

    if (!file)
    {
        return -1;
    }

    file.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size())
    );
    file.close();

    if (!file)
    {
        return -2;
    }

    return 0;
}
