//
// Created by NHCM on 2026/11/22.
//

#include "JavaCompiler.h"

#include "Compiler.h"
#include "cli/ConfigParser.h"
#include "util/DataUtil.h"
#include "util/PrintUtil.h"

int main(int argc, char** argv)
{
    if (argc <= 1)
    {
        jcutils::printlnErrorBy("Missing options");
        return -1;
    }

    for (int i = 1; i < argc; i++)
    {
        javac::args.emplace_back(argv[i]);
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
            jcutils::println(info.name, jcutils::PrintColor::None);
        }
        return 0;
    }

    javac::JavacConfig* parsedConfig = nullptr;
    try
    {
        parsedConfig = javac::ConfigParser::parseConfig(javac::args);
        javac::config = parsedConfig;
    } catch (std::invalid_argument& e)
    {
        delete parsedConfig;
        jcutils::printlnErrorBy(e.what());
        return -1;
    }

    int exitCode = javac::Compiler::run();

    delete javac::config;

    return exitCode;
}
