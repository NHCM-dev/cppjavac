//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_CONFIGPARSER_H
#define JAVACOMPILER_CONFIGPARSER_H

#include "main/core/JavacConfig.h"

namespace javac
{
    class ConfigParser
    {
    private:
        static std::set<DebugInfo> parseDebugInfo(std::string_view value);

    public:
        /**
         * Parse running config for javac
         * @throws std::invalid_argument If any argument is invalid
         * @param args user args entered into javac cli
         * @return if args are valid, returns valid JavacConfig pointer, otherwise returns nullptr
         */
        static JavacConfig* parseConfig(const std::vector<std::string>& args);
    };

} // javac

#endif //JAVACOMPILER_CONFIGPARSER_H
