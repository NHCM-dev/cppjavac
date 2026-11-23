//
// Created by NHCM on 2026/11/22.
//

#include "ConfigParser.h"

#include "main/util/DataUtil.h"
#include "main/util/MathUtil.h"

namespace javac
{
    JavacConfig* ConfigParser::parseConfig(const std::vector<std::string>& args)
    {
        if (args.empty())
        {
            throw std::invalid_argument("No arguments given");
        }
        if (jcutils::contains(args, "-h") || jcutils::contains(args, "--help")
            || jcutils::contains(args, "--version") || jcutils::contains(args, "--list-extensions"))
        {
            return nullptr;
        }

        bool outputDirGiven = false;
        bool genDebugGiven = false;
        bool sourceLangGiven = false;
        bool targetLangGiven = false;
        bool encodingGiven = false;

        std::optional<JavacAction> action;
        std::vector<Path> srcPaths = {};
        Path outputDir = ".";
        std::vector<Path> classPaths = {};
        TextEncoding encoding = TextEncoding::UTF8;
        std::set debugs = {DebugInfo::Lines, DebugInfo::Vars, DebugInfo::Source};
        LangLevel sourceLangLevel = LangLevel::Eight;
        LangLevel targetLangLevel = LangLevel::Eight;
        bool verbose = false;
        bool nowarn = false;
        std::set<JavacExtension> extensions;

        for (std::size_t i = 0; i < args.size(); ++i)
        {
            const std::string& arg = args[i];
            const auto argIs = [&arg](auto... spellings) {
                return jcutils::matches(arg, spellings...);
            };

            bool compile = argIs("-c", "--compile");
            bool inspect = argIs("-i", "--inspect");
            bool lexer = argIs("-l", "--lexer");

            if (compile || inspect || lexer)
            {
                if (action)
                {
                    throw std::invalid_argument("Duplicate compiler action given");
                }
                action = compile ? JavacAction::Compile : (inspect ? JavacAction::Inspect : JavacAction::Lexer);
                continue;
            }

            const bool hasNextArg = i + 1 < args.size() && !args[i + 1].starts_with('-');
            const std::string* nextArg = hasNextArg ? &args[i + 1] : nullptr;
            if (arg.starts_with("-ex:") || arg.starts_with("--extension:"))
            {
                extensions.merge(parseExtensions(std::string_view(arg).substr(arg.find(':') + 1)));
                continue;
            }
            if (argIs("-s", "--source-path"))
            {
                if (!hasNextArg)
                {
                    throw std::invalid_argument("No source path given");
                }
                Path srcPath = {*nextArg};
                srcPaths.emplace_back(srcPath);
                i++;
                continue;
            }
            if (argIs("-o", "--output-path"))
            {
                if (outputDirGiven)
                {
                    throw std::invalid_argument("Duplicate output path given");
                }
                if (!hasNextArg)
                {
                    throw std::invalid_argument("No output path given");
                }
                outputDir = {*nextArg};
                outputDirGiven = true;
                i++;
                continue;
            }
            if (argIs("-cp", "--class-path", "--classpath"))
            {
                if (!hasNextArg)
                {
                    throw std::invalid_argument("No classpath given");
                }
                Path cp = {*nextArg};
                classPaths.emplace_back(cp);
                i++;
                continue;
            }
            const bool allDebug = argIs("-g", "--generate-debug");
            if (allDebug || arg.starts_with("-g:") || arg.starts_with("--generate-debug:"))
            {
                if (genDebugGiven)
                {
                    throw std::invalid_argument("Duplicate debug option given");
                }
                if (!allDebug)
                {
                    debugs = parseDebugInfo(std::string_view(arg).substr(arg.find(':') + 1));
                }
                genDebugGiven = true;
                continue;
            }
            if (argIs("-v", "--verbose"))
            {
                verbose = true;
                continue;
            }

            if (argIs("--nowarn"))
            {
                nowarn = true;
                continue;
            }

            if (argIs("--source"))
            {
                if (sourceLangGiven)
                {
                    throw std::invalid_argument("Duplicate source language level given");
                }
                if (!hasNextArg)
                {
                    throw std::invalid_argument("No source language level given");
                }
                int langLevel;
                try
                {
                    langLevel = jcutils::parseInt(*nextArg);
                } catch (const std::invalid_argument& e)
                {
                    throw std::invalid_argument("Cannot parse \"" + *nextArg + "\" as integer language level");
                }
                std::optional<LangLevel> parsedLangLevel = parseLangLevel(langLevel);
                if (!parsedLangLevel)
                {
                    throw std::invalid_argument("Language level \"" + *nextArg + "\" is not supported");
                }
                sourceLangLevel = parsedLangLevel.value();
                sourceLangGiven = true;
                i++;
                continue;
            }

            if (argIs("--target"))
            {
                if (targetLangGiven)
                {
                    throw std::invalid_argument("Duplicate target language level given");
                }
                if (!hasNextArg)
                {
                    throw std::invalid_argument("No target language level given");
                }
                int langLevel;
                try
                {
                    langLevel = jcutils::parseInt(*nextArg);
                } catch (const std::invalid_argument& e)
                {
                    throw std::invalid_argument("Cannot parse \"" + *nextArg + "\" as integer language level");
                }
                std::optional<LangLevel> parsedLangLevel = parseLangLevel(langLevel);
                if (!parsedLangLevel)
                {
                    throw std::invalid_argument("Language level \"" + *nextArg + "\" is not supported");
                }
                targetLangLevel = parsedLangLevel.value();
                targetLangGiven = true;
                i++;
                continue;
            }

            if (argIs("--encoding"))
            {
                if (encodingGiven)
                {
                    throw std::invalid_argument("Duplicate source encoding specification given");
                }
                if (!hasNextArg)
                {
                    throw std::invalid_argument("No source encoding specification given");
                }
                std::optional<TextEncoding> parsedEncoding = parseEncoding(*nextArg);
                if (!parsedEncoding)
                {
                    throw std::invalid_argument("Cannot parse \"" + *nextArg + "\" as any text encoding");
                }

                encoding = parsedEncoding.value();
                encodingGiven = true;
                i++;
                continue;
            }

            if (arg.starts_with('-'))
            {
                throw std::invalid_argument("Invalid argument \"" + arg + "\"");
            }

            Path parsePath = arg;
            srcPaths.push_back(parsePath);
        }

        if (!action)
        {
            throw std::invalid_argument("Action is missing");
        }

        return new JavacConfig(*action, srcPaths, outputDir, classPaths, encoding, debugs,
            sourceLangLevel, targetLangLevel, verbose, nowarn, std::move(extensions));
    }

    std::set<DebugInfo> ConfigParser::parseDebugInfo(std::string_view value)
    {
        std::set<DebugInfo> result;

        if (value == "none")
        {
            return result;
        }

        if (value.empty())
        {
            throw std::invalid_argument("Empty debug option");
        }

        while (!value.empty())
        {
            const size_t pos = value.find(',');
            const std::string_view token = value.substr(0, pos);

            if (token == "lines")
            {
                result.insert(DebugInfo::Lines);
            }
            else if (token == "vars")
            {
                result.insert(DebugInfo::Vars);
            }
            else if (token == "source")
            {
                result.insert(DebugInfo::Source);
            }
            else
            {
                throw std::invalid_argument("Invalid debug option: " + std::string(token));
            }

            if (pos == std::string_view::npos)
            {
                break;
            }

            value.remove_prefix(pos + 1);
            if (value.empty())
            {
                throw std::invalid_argument("Empty debug option after comma");
            }
        }

        return result;
    }
} // javac
