//
// Created by NHCM on 2026/11/22.
//

#include "JavaCompiler.h"

#include "util/DataSetUtil.h"
#include "util/PrintUtil.h"

int main(int argc, char** argv)
{
    if (argc <= 1)
    {
        jcutils::print(javac::helpDoc);
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        javac::args.emplace_back(argv[i]);
    }

    if (jcutils::contains(javac::args, {"-h"})
        || jcutils::contains(javac::args, {"--help"}))
    {
        jcutils::print(javac::helpDoc);
        return 1;
    }

    return 0;
}
