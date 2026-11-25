//
// Created by NHCM on 2026/11/23.
//

#include "Parser.h"

namespace javac
{
    CompilationUnitType Parser::decideType(const Path& filePath)
    {
        std::string fileName = filePath.filename().string();
        if (fileName == "module-info.java")
        {
            return CompilationUnitType::Modular;
        }
        return CompilationUnitType::Ordinary;
    }

    CompilationUnit Parser::parseModular()
    {
        CompilationUnit compilationUnit;
        compilationUnit.unitType = CompilationUnitType::Modular;
        return compilationUnit;
    }
    CompilationUnit Parser::parseOrdinary()
    {
        CompilationUnit compilationUnit;
        compilationUnit.unitType = CompilationUnitType::Ordinary;

        for (const auto& token : inputStream.tokens)
        {
            switch (token.tokenType)
            {

            }
        }
        return compilationUnit;
    }

    CompilationUnit Parser::parse()
    {
        CompilationUnitType compilationUnitType = decideType(this->filePath);
        switch (compilationUnitType)
        {
            case CompilationUnitType::Modular:
                return parseModular();
            default:
                return parseOrdinary();
        }
    }
} // javac
