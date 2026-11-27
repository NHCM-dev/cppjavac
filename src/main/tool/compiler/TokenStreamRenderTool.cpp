//
// Created by NHCM on 2026/11/27.
//

#include "TokenStreamRenderTool.h"

#include "RenderText.h"
#include "main/tool/EncodingTool.h"

namespace
{
    using namespace javac;

    std::string_view enumName(TokenType value)
    {
        switch (value)
        {
            case TokenType::EndOfFile: return "EndOfFile";
            case TokenType::Invalid: return "Invalid";
            case TokenType::Identifier: return "Identifier";
            case TokenType::Keyword: return "Keyword";
            case TokenType::IntegerLiteral: return "IntegerLiteral";
            case TokenType::FloatLiteral: return "FloatLiteral";
            case TokenType::CharacterLiteral: return "CharacterLiteral";
            case TokenType::StringLiteral: return "StringLiteral";
            case TokenType::BooleanLiteral: return "BooleanLiteral";
            case TokenType::NullLiteral: return "NullLiteral";
            case TokenType::Symbol: return "Symbol";
            case TokenType::LineComment: return "LineComment";
            case TokenType::BlockComment: return "BlockComment";
            case TokenType::JavadocComment: return "JavadocComment";
            case TokenType::NewLine: return "NewLine";
            case TokenType::Whitespace: return "Whitespace";
        }
        return "<unknown>";
    }

    std::string_view enumName(SymbolType value)
    {
        switch (value)
        {
            case SymbolType::LParen: return "LParen";
            case SymbolType::RParen: return "RParen";
            case SymbolType::LBrace: return "LBrace";
            case SymbolType::RBrace: return "RBrace";
            case SymbolType::LBracket: return "LBracket";
            case SymbolType::RBracket: return "RBracket";
            case SymbolType::Question: return "Question";
            case SymbolType::Colon: return "Colon";
            case SymbolType::Semicolon: return "Semicolon";
            case SymbolType::Comma: return "Comma";
            case SymbolType::Dot: return "Dot";
            case SymbolType::Ellipsis: return "Ellipsis";
            case SymbolType::AtSign: return "AtSign";
            case SymbolType::DoubleColon: return "DoubleColon";
            case SymbolType::Underscore: return "Underscore";
            case SymbolType::Plus: return "Plus";
            case SymbolType::Minus: return "Minus";
            case SymbolType::Star: return "Star";
            case SymbolType::Slash: return "Slash";
            case SymbolType::Percent: return "Percent";
            case SymbolType::PlusPlus: return "PlusPlus";
            case SymbolType::MinusMinus: return "MinusMinus";
            case SymbolType::Assign: return "Assign";
            case SymbolType::PlusAssign: return "PlusAssign";
            case SymbolType::MinusAssign: return "MinusAssign";
            case SymbolType::StarAssign: return "StarAssign";
            case SymbolType::SlashAssign: return "SlashAssign";
            case SymbolType::PercentAssign: return "PercentAssign";
            case SymbolType::AmpersandAssign: return "AmpersandAssign";
            case SymbolType::PipeAssign: return "PipeAssign";
            case SymbolType::CaretAssign: return "CaretAssign";
            case SymbolType::LeftShiftAssign: return "LeftShiftAssign";
            case SymbolType::RightShiftAssign: return "RightShiftAssign";
            case SymbolType::UnsignedRightShiftAssign: return "UnsignedRightShiftAssign";
            case SymbolType::EqualEqual: return "EqualEqual";
            case SymbolType::BangEqual: return "BangEqual";
            case SymbolType::LAngleBracket: return "LAngleBracket";
            case SymbolType::LessEqual: return "LessEqual";
            case SymbolType::RAngleBracket: return "RAngleBracket";
            case SymbolType::GreaterEqual: return "GreaterEqual";
            case SymbolType::Bang: return "Bang";
            case SymbolType::AmpersandAmpersand: return "AmpersandAmpersand";
            case SymbolType::PipePipe: return "PipePipe";
            case SymbolType::Ampersand: return "Ampersand";
            case SymbolType::Pipe: return "Pipe";
            case SymbolType::Caret: return "Caret";
            case SymbolType::Tilde: return "Tilde";
            case SymbolType::LeftShift: return "LeftShift";
            case SymbolType::RightShift: return "RightShift";
            case SymbolType::UnsignedRightShift: return "UnsignedRightShift";
            case SymbolType::Arrow: return "Arrow";
        }
        return "<unknown>";
    }

    std::string_view enumName(JavaKeywordKind value)
    {
        switch (value)
        {
            case JavaKeywordKind::Reserved: return "Reserved";
            case JavaKeywordKind::ReservedUnused: return "ReservedUnused";
            case JavaKeywordKind::Contextual: return "Contextual";
            case JavaKeywordKind::BooleanLiteral: return "BooleanLiteral";
            case JavaKeywordKind::NullLiteral: return "NullLiteral";
        }
        return "<unknown>";
    }
}

std::string TokenStreamRenderTool::render(const TokenStream& stream)
{
    std::string output;
    for (const auto& token : stream.tokens)
    {
        output += sourceLocationToString(token.range.begin) + ' ';
        output += enumName(token.tokenType);
        output += ' ' + render::quoteUtf16(token.content) + ' ' + render::rangeText(token.range);

        if (token.symbol)
        {
            output += " symbol=";
            output += enumName(*token.symbol);
        }
        if (token.keyword)
        {
            output += " keyword=";
            output += token.keyword->getString();
            output += " kind=";
            output += enumName(token.keyword->getKind());
            output += " since=" + std::to_string(static_cast<unsigned>(token.keyword->getMinLangLevel()));
        }
        if (token.canBeIdentifier)
        {
            output += " canBeIdentifier";
        }
        if (token.invalidReason)
        {
            output += " (";
            output += invalidTokenStr(*token.invalidReason);
            output += ')';
        }
        output += '\n';
    }

    // Invalid tokens already occur in the main token sequence.
    output += "\nInvalid tokens: " + std::to_string(stream.invalidTokens.size()) + '\n';
    if (!stream.sourceOffsets.empty())
    {
        output += "\nTranslated UTF-16 offsets to original UTF-16 offsets:\n";
        for (std::size_t i = 0; i < stream.sourceOffsets.size(); ++i)
        {
            output += std::to_string(i) + " -> " + std::to_string(stream.sourceOffsets[i]) + '\n';
        }
    }
    return output;
}

std::u16string TokenStreamRenderTool::renderListing(std::u16string_view source, const TokenStream& stream)
{
    std::u16string output(source);
    output += u"\n\n===Lexer tokens===\n\n";
    output += EncodingTool::utf8ToUtf16(render(stream));
    return output;
}
