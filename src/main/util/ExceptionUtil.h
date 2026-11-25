//
// Created by NHCM on 2026/11/24.
//

#ifndef CPPJAVAC_EXCEPTIONUTIL_H
#define CPPJAVAC_EXCEPTIONUTIL_H
#include <optional>
#include <string_view>

#include "main/core/DataDef.h"

namespace jcutils
{
    enum class DiagnosticSeverity
    {
        Error,
        Warning,
        Note,
        Remark,
        FatalError,
    };

    /**
     * Source position or range to mark in a diagnostic.
     * Columns are 1-based and measured in UTF-16 code units.
     * The range includes begin and excludes end. Omit end to mark a single position.
     */
    struct DiagnosticSite
    {
        SourceLocation begin;
        std::optional<SourceLocation> end;

        DiagnosticSite(SourceLocation begin, std::optional<SourceLocation> end = std::nullopt)
        : begin(begin), end(end)
        {}
    };

    /**
     * Print a diagnostic to stderr with an optional marked source excerpt.
     * @param severity diagnostic level
     * @param path optional source file path
     * @param message diagnostic text
     * @param site optional source position or range to mark
     * @param sourceFile optional full decoded UTF-16 source before Unicode escape translation.
     * When supplied, site offsets determine the displayed line and column
     * @param owner optional diagnostic source
     */
    void printDiagnostic(
        DiagnosticSeverity severity,
        std::optional<Path> path,
        std::string_view message,
        std::optional<DiagnosticSite> site = std::nullopt,
        std::optional<std::u16string_view> sourceFile = std::nullopt,
        std::string_view owner = {});
} // jcutils

#endif //CPPJAVAC_EXCEPTIONUTIL_H
