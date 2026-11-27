//
// Created by NHCM on 2026/11/27.
//

#ifndef CPPJAVAC_TOKENSTREAMRENDERTOOL_H
#define CPPJAVAC_TOKENSTREAMRENDERTOOL_H

#include <string>
#include <string_view>

#include "main/lexer/LexerDef.h"

class TokenStreamRenderTool
{
public:
    // Render tokens as UTF-8, including trivia and end of file.
    [[nodiscard]] static std::string render(const javac::TokenStream& stream);

    // Render the original input followed by the token listing for a .javal file.
    [[nodiscard]] static std::u16string renderListing(std::u16string_view source, const javac::TokenStream& stream);
};


#endif //CPPJAVAC_TOKENSTREAMRENDERTOOL_H
