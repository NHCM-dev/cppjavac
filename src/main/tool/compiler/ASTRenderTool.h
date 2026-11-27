//
// Created by NHCM on 2026/11/27.
//

#ifndef CPPJAVAC_ASTRENDERTOOL_H
#define CPPJAVAC_ASTRENDERTOOL_H

#include <string>

#include "main/parser/ast/astdefs/Declarations.h"

class ASTRenderTool
{
public:
    // Render a compilation unit as a UTF-8 tree.
    [[nodiscard]] static std::string render(const Path& file, const javac::CompilationUnit& unit);
};


#endif //CPPJAVAC_ASTRENDERTOOL_H
