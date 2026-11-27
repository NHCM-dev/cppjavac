//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_DATADEF_H
#define JAVACOMPILER_DATADEF_H

#include <filesystem>
#include <vector>
#include <string>
#include <string_view>
#include <utility>

#include "def/TextEncoding.h"
#include "def/JavaDef.h"

namespace fs = std::filesystem;

using ByteArray = std::vector<std::byte>;

template<typename T>
using Uptr = std::unique_ptr<T>;

template<typename T, typename... Args>
constexpr auto makeUniq(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

/**
 * A class that inherit std::filesystem::path, will convert value constructed into UTF8 string
 */
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

struct SourceLocation
{
    std::size_t offsetInFile = 0; // UTF-16 offset before Unicode escape translation
    // Line and column refer to the translated input.
    std::size_t line = 1;
    std::size_t column = 1;
};

struct SourceRange
{
    SourceLocation begin;
    SourceLocation end; // Exclusive
};

inline std::string sourceLocationToString(const SourceLocation& sourceLocation)
{
    return std::string(std::to_string(sourceLocation.line) + ":" + std::to_string(sourceLocation.column));
}

#endif //JAVACOMPILER_DATADEF_H
