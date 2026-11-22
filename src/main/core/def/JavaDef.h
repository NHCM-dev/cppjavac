//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_JAVADEF_H
#define JAVACOMPILER_JAVADEF_H

namespace javac
{
    enum class DebugInfo
    {
        Lines, // Line number
        Vars, // Local variable
        Source // Source file
    };

    enum class LangLevel : uint8_t
    {
        Eight = 8
    };

    inline std::optional<LangLevel> parseLangLevel(int level)
    {
        switch (level)
        {
            case static_cast<int>(LangLevel::Eight):
                return LangLevel::Eight;
            default:
                return std::nullopt;
        }
    }
}

#endif //JAVACOMPILER_JAVADEF_H
