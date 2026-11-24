//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_DATADEF_H
#define JAVACOMPILER_DATADEF_H

#include <filesystem>
#include <vector>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "def/TextEncoding.h"
#include "def/JavaDef.h"

namespace fs = std::filesystem;

using ByteArray = std::vector<std::byte>;

class Path : public fs::path
{
public:
    Path() = default;
    Path(path value) : path(std::move(value)) {}

    // Narrow strings at the project boundary are UTF-8.
    Path(std::string_view utf8) : path(std::u8string(utf8.begin(), utf8.end())) {}
    Path(const std::string& utf8) : Path(std::string_view(utf8)) {}
    Path(const char* utf8) : Path(std::string_view(utf8)) {}

    [[nodiscard]] const path& fsPath() const noexcept { return *this; }

    [[nodiscard]] std::string string() const
    {
        const auto utf8 = u8string();
        return {utf8.begin(), utf8.end()};
    }

    [[nodiscard]] Path filename() const { return path::filename(); }
    [[nodiscard]] Path extension() const { return path::extension(); }
    [[nodiscard]] Path parent_path() const { return path::parent_path(); }
    [[nodiscard]] Path lexically_normal() const { return path::lexically_normal(); }
    [[nodiscard]] Path lexically_relative(const path& base) const { return path::lexically_relative(base); }

    friend Path operator/(const Path& left, const Path& right)
    {
        return left.fsPath() / right.fsPath();
    }
};

#endif //JAVACOMPILER_DATADEF_H
