//
// Created by NHCM on 2026/11/22.
//

#include "cppjavac.h"

#include "Compiler.h"
#include "cli/ConfigParser.h"
#include "util/DataUtil.h"
#include "util/PrintUtil.h"

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif
{
    if (argc <= 1)
    {
        jcutils::printlnErrorBy("Missing options");
        return -1;
    }

    for (int i = 1; i < argc; i++)
    {
        #ifdef _WIN32
        javac::args.emplace_back(Path(fs::path(argv[i])).string());
        #else
        javac::args.emplace_back(argv[i]);
        #endif
    }

    if (jcutils::contains(javac::args, "-h")
        || jcutils::contains(javac::args, "--help"))
    {
        jcutils::print(javac::helpDoc);
        return 1;
    }

    if (jcutils::contains(javac::args, "--version"))
    {
        jcutils::print(javac::version);
        return 1;
    }

    if (jcutils::contains(javac::args, "--list-extensions"))
    {
        jcutils::printlnInfoBy("List of currently supported compiler extensions");
        for (const auto& info : javac::listExtensions())
        {
            jcutils::println(info.name);
        }
        return 0;
    }

    javac::JavacConfig* parsedConfig = nullptr;
    try
    {
        parsedConfig = javac::ConfigParser::parseConfig(javac::args);
        javac::config = parsedConfig;
    } catch (const fs::filesystem_error& e)
    {
        jcutils::printlnErrorBy(e.what());
        return -1;
    } catch (std::invalid_argument& e)
    {
        delete parsedConfig;
        jcutils::printlnErrorBy(e.what());
        return -1;
    }

    javac::compiler = new javac::Compiler();
    int exitCode = javac::compiler->run();

    delete javac::compiler;
    delete javac::config;

    return exitCode;
}
