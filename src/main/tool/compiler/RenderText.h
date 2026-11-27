#ifndef CPPJAVAC_RENDERTEXT_H
#define CPPJAVAC_RENDERTEXT_H

#include "main/core/DataDef.h"

namespace javac::render
{
    inline std::string quoteUtf16(std::u16string_view text, char quote = '"')
    {
        constexpr char hex[] = "0123456789ABCDEF";
        std::string result(1, quote);
        for (const char16_t ch : text)
        {
            if (ch == static_cast<unsigned char>(quote) || ch == u'\\')
            {
                result += '\\';
                result += static_cast<char>(ch);
                continue;
            }

            switch (ch)
            {
                case u'\n': result += "\\n"; break;
                case u'\r': result += "\\r"; break;
                case u'\t': result += "\\t"; break;
                default:
                    if (ch >= u' ' && ch <= u'~')
                    {
                        result += static_cast<char>(ch);
                    }
                    else
                    {
                        result += "\\u";
                        for (int shift = 12; shift >= 0; shift -= 4)
                        {
                            result += hex[(ch >> shift) & 0xF];
                        }
                    }
            }
        }
        result += quote;
        return result;
    }

    inline std::string rangeText(const SourceRange& range)
    {
        return '[' + sourceLocationToString(range.begin) + ".." + sourceLocationToString(range.end)
            + ", UTF-16 " + std::to_string(range.begin.offsetInFile) + ".."
            + std::to_string(range.end.offsetInFile) + ')';
    }
} // javac::render

#endif // CPPJAVAC_RENDERTEXT_H
