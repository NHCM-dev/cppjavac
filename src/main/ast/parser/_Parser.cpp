//
// Created by NHCM on 2026/11/24.
//

#include "Parser.h"

namespace javac
{
    void Parser::errorProcessPlaceholder()
    {

    }

    CompilationUnitType Parser::decideType(const Path& filePath)
    {
        std::string fileName = filePath.filename().string();
        if (fileName == "module-info.java")
        {
            return CompilationUnitType::Modular;
        }
        return CompilationUnitType::Ordinary;
    }




    bool Parser::isOp(ParserOp operation)
    {
        return this->op ==  operation;
    }

    void Parser::setOp(ParserOp operation)
    {
        this->op = operation;
    }

    void Parser::clearOp()
    {
        this->op = ParserOp::None;
    }

    bool Parser::opUnfinished()
    {
        return this->op == ParserOp::PendingAnnotation
        || this->op == ParserOp::PendingPackage;
    }




    void Parser::appendBuffer(uint8_t slot, std::u16string_view string)
    {
        this->stringBufferSlots[slot] += string;
    }

    void Parser::appendBuffer(uint8_t slot, char16_t character)
    {
        this->stringBufferSlots[slot] += character;
    }

    const std::u16string& Parser::readBuffer(uint8_t slot)
    {
        return this->stringBufferSlots.at(slot);
    }

    std::u16string Parser::flushBuffer(uint8_t slot)
    {
        return std::exchange(this->stringBufferSlots.at(slot), {});
    }

    // bool Parser::bufferHasSpace(uint8_t slot)
    // {
    //     return jcutils::hasSpace(this->stringBufferSlots.at(slot));
    // }


    void Parser::pushAnnotation(Annotation annotation)
    {
        this->annotationBuffer.push_back(std::move(annotation));
    }

    std::optional<Annotation> Parser::popAnnotation()
    {
        if (this->annotationBuffer.empty())
        {
            return std::nullopt;
        }

        Annotation ret = std::move(this->annotationBuffer.back());
        this->annotationBuffer.pop_back();

        return ret;
    }

    std::queue<Annotation> Parser::flushAnnotation()
    {
        return std::queue<Annotation>(
            std::exchange(this->annotationBuffer, {})
        );
    }

    std::size_t Parser::delimiterCount(SymbolType type)
    {
        return delimiterCounter(type);
    }

    std::size_t& Parser::delimiterCounter(SymbolType opening)
    {
        switch (opening)
        {
            case SymbolType::LParen: return unusedParen;
            case SymbolType::LBracket: return unusedBracket;
            case SymbolType::LBrace: return unusedBrace;
            default: throw std::invalid_argument("Expected an opening delimiter");
        }
    }

    void Parser::pushDelimiter(SymbolType opening)
    {
        ++delimiterCounter(opening);
        delimiterOrder.push(opening);
    }

    void Parser::popDelimiter(SymbolType opening)
    {
        if (delimiterOrder.empty() || delimiterOrder.top() != opening)
        {
            errorProcessPlaceholder();
            return;
        }

        --delimiterCounter(opening);
        delimiterOrder.pop();
    }

    void Parser::clearDelimiters()
    {
        unusedParen = 0;
        unusedBracket = 0;
        unusedBrace = 0;
        delimiterOrder = {};
    }

    PackageDecl& Parser::getOrCreatePackage()
    {
        if (!compilationUnit.packageDecl)
        {
            compilationUnit.packageDecl = {};
            return compilationUnit.packageDecl.value();
        }
        return compilationUnit.packageDecl.value();
    }

}