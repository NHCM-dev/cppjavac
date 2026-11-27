//
// Created by NHCM on 2026/11/25.
//

#include "ParserUtil.h"

#include <algorithm>
#include <bit>
#include <charconv>
#include <limits>
#include <set>

#include "main/tool/EncodingTool.h"
#include "main/util/MathUtil.h"

namespace jcutils
{
    static bool hasLongSuffix(std::u16string_view str)
    {
        return str.ends_with(u'l') || str.ends_with(u'L');
    }

    static std::string normalizeNumber(std::u16string_view str)
    {
        auto number = EncodingTool::utf16ToUtf8(str);
        std::erase(number, '_');
        return number;
    }

    javac::jlong int64FromString(std::u16string_view str)
    {
        const bool negative = str.starts_with(u'-');
        if (negative || str.starts_with(u'+'))
        {
            str.remove_prefix(1);
        }
        if (!isInteger(str))
        {
            throw std::invalid_argument("Invalid integer literal");
        }

        const bool wide = hasLongSuffix(str);
        if (wide)
        {
            str.remove_suffix(1);
        }

        int base = 10;
        if (str.starts_with(u"0x") || str.starts_with(u"0X"))
        {
            base = 16;
            str.remove_prefix(2);
        }
        else if (str.starts_with(u"0b") || str.starts_with(u"0B"))
        {
            base = 2;
            str.remove_prefix(2);
        }
        else if (str.size() > 1 && str.front() == u'0')
        {
            base = 8;
        }

        const auto number = normalizeNumber(str);
        const auto end = number.data() + number.size();
        std::uint64_t magnitude = 0;
        const auto result = std::from_chars(number.data(), end, magnitude, base);
        const std::uint64_t signedMax = wide
            ? std::numeric_limits<std::int64_t>::max() : std::numeric_limits<std::int32_t>::max();
        const std::uint64_t limit = base == 10 ? signedMax + negative
            : (wide ? std::numeric_limits<std::uint64_t>::max() : std::numeric_limits<std::uint32_t>::max());
        if (result.ec == std::errc::result_out_of_range || magnitude > limit)
        {
            throw std::out_of_range("Integer literal out of range");
        }
        if (result.ec != std::errc{} || result.ptr != end)
        {
            throw std::invalid_argument("Invalid integer literal");
        }

        // Non-decimal literals use the entire bit width of their declared type.
        const auto bits = negative ? std::uint64_t{0} - magnitude : magnitude;
        return wide ? std::bit_cast<std::int64_t>(bits)
            : std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(bits));
    }

    bool int64IsLong(javac::jlong val, std::u16string_view spelling)
    {
        if (!spelling.empty())
        {
            return hasLongSuffix(spelling);
        }
        return val < std::numeric_limits<std::int32_t>::min()
            || val > std::numeric_limits<std::int32_t>::max();
    }

    template<typename Float>
    static Float floatingFromString(std::u16string_view str)
    {
        const bool negative = str.starts_with(u'-');
        if (negative || str.starts_with(u'+'))
        {
            str.remove_prefix(1);
        }

        const bool decimalInteger = !str.empty() && std::ranges::all_of(str, [](char16_t ch)
        {
            return ch >= u'0' && ch <= u'9';
        });
        if (!decimalInteger && !isFloat(str))
        {
            throw std::invalid_argument("Invalid floating-point number");
        }

        if (str.ends_with(u'f') || str.ends_with(u'F') || str.ends_with(u'd') || str.ends_with(u'D'))
        {
            str.remove_suffix(1);
        }

        const bool hex = str.starts_with(u"0x") || str.starts_with(u"0X");
        if (hex)
        {
            str.remove_prefix(2);
        }
        const auto number = normalizeNumber(str);
        Float value;
        const auto end = number.data() + number.size();
        const auto result = std::from_chars(number.data(), end, value,
            hex ? std::chars_format::hex : std::chars_format::general);
        if (result.ec == std::errc::result_out_of_range)
        {
            throw std::out_of_range("Floating-point literal out of range");
        }
        if (result.ec != std::errc{} || result.ptr != end)
        {
            throw std::invalid_argument("Invalid floating-point number");
        }
        return negative ? -value : value;
    }

    javac::jfloat floatFromString(std::u16string_view str)
    {
        return floatingFromString<javac::jfloat>(str);
    }

    javac::jdouble doubleFromString(std::u16string_view str)
    {
        return floatingFromString<javac::jdouble>(str);
    }

    bool is32Float(std::u16string_view str)
    {
        return (str.ends_with(u'f') || str.ends_with(u'F')) && isFloat(str);
    }

    javac::jboolean boolFromString(std::u16string_view input)
    {
        return input == u"true";
    }

    bool isActualClass(const javac::TypeNode& typeNode)
    {
        return typeNode.objectType == javac::ObjectType::Class
            || typeNode.objectType == javac::ObjectType::GenericClass;
    }

    bool isPrimitive(const javac::TypeNode& typeNode)
    {
        return typeNode.objectType == javac::ObjectType::Primitive;
    }

    bool canExtImplTo(const javac::TypeNode& typeNode)
    {
        return isActualClass(typeNode);
    }

    bool canPermit(const javac::TypeNode& typeNode)
    {
        return typeNode.objectType == javac::ObjectType::Class
            && std::ranges::none_of(typeNode.nameParts, [](const javac::TypeNamePart& part)
            {
                return !part.annotations.empty();
            });
    }

    bool canGeneric(const javac::TypeNode& typeNode)
    {
        return isActualClass(typeNode) || typeNode.objectType == javac::ObjectType::Array;
    }

    bool haveModifier(const std::vector<javac::ModifierFlag>& modifiers, javac::FlagType flag)
    {
        return std::ranges::any_of(modifiers, [flag](const auto& modifier)
        {
            return modifier.flagType == flag;
        });
    }

    const javac::ModifierFlag* findInvalidMethodModifier(const std::vector<javac::ModifierFlag>& modifiers,
        MethodDeclContext context, javac::LangLevel langLevel)
    {
        using enum javac::FlagType;
        static constexpr javac::FlagType normal[] = {
            Public, Protected, Private, Abstract, Static, Final, Native, Synchronized, Strictfp
        };

        const auto invalid = std::ranges::find_if_not(modifiers, [&](const auto& modifier)
        {
            if (!modifier.flagType)
            {
                return true;
            }
            const auto flag = *modifier.flagType;
            switch (context)
            {
            case MethodDeclContext::Normal:
            case MethodDeclContext::WithinAbstractClass:
                return contains(normal, flag);
            case MethodDeclContext::WithinInterface:
                return flag == Public || flag == Abstract || flag == Default || flag == Static || flag == Strictfp
                    || (flag == Private && langLevel >= javac::LangLevel::Nine);
            case MethodDeclContext::WithinAnnotation:
                return flag == Public || flag == Abstract;
            }
            return false;
        });
        return invalid == modifiers.end() ? nullptr : &*invalid;
    }

    const javac::ModifierFlag* findInvalidClassModifier(const std::vector<javac::ModifierFlag>& modifiers, javac::LangLevel langLevel,
        ClassDeclContext context)
    {
        using enum javac::FlagType;
        const auto invalid = std::ranges::find_if_not(modifiers, [&](const auto& modifier)
        {
            if (!modifier.flagType)
            {
                return true;
            }
            switch (*modifier.flagType)
            {
            case Abstract:
            case Final:
            case Strictfp:
                return true;
            case Public:
                return context != ClassDeclContext::Local;
            case Protected:
            case Private:
                return context == ClassDeclContext::Member;
            case Static:
                return context != ClassDeclContext::TopLevel && context != ClassDeclContext::Local;
            case Sealed:
            case NonSealed:
                return context != ClassDeclContext::Local && langLevel >= javac::LangLevel::Seventeen;
            default:
                return false;
            }
        });
        return invalid == modifiers.end() ? nullptr : &*invalid;
    }

    const javac::ModifierFlag* findInvalidFieldModifier(const std::vector<javac::ModifierFlag>& modifiers, ClassDeclContext context)
    {
        static constexpr javac::FlagType allowed[] = {
            javac::FlagType::Public,
            javac::FlagType::Protected,
            javac::FlagType::Private,
            javac::FlagType::Static,
            javac::FlagType::Final,
            javac::FlagType::Transient,
            javac::FlagType::Volatile
        };
        const auto invalid = std::ranges::find_if_not(modifiers, [&](const auto& modifier)
        {
            if (!modifier.flagType.has_value())
            {
                return true;
            }
            const auto flag = *modifier.flagType;
            if (isInterfaceBody(context))
            {
                return flag == javac::FlagType::Public || flag == javac::FlagType::Static || flag == javac::FlagType::Final;
            }
            return contains(allowed, flag);
        });
        return invalid == modifiers.end() ? nullptr : &*invalid;
    }

    const javac::ModifierFlag* findInvalidConstructorModifier(const std::vector<javac::ModifierFlag>& modifiers)
    {
        static constexpr javac::FlagType allowed[] = {
            javac::FlagType::Public,
            javac::FlagType::Protected,
            javac::FlagType::Private,
        };
        const auto invalid = std::ranges::find_if_not(modifiers, [&](const auto& modifier)
        {
            if (!modifier.flagType.has_value())
            {
                return true;
            }
            return contains(allowed, modifier.flagType.value());
        });
        return invalid == modifiers.end() ? nullptr : &*invalid;
    }

    std::optional<ModifierConflict> findModifierConflict(
        const std::vector<javac::ModifierFlag>& modifiers, ModifierDeclaration declaration)
    {
        using javac::FlagType;
        using javac::ParserExceptionType;

        std::set<FlagType> flags;
        const auto declarationConflict = [&]() -> std::optional<ParserExceptionType>
        {
            switch (declaration)
            {
            case ModifierDeclaration::Class:
                if ((flags.contains(FlagType::Abstract) && flags.contains(FlagType::Final))
                    || flags.count(FlagType::Sealed) + flags.count(FlagType::NonSealed) + flags.count(FlagType::Final) > 1)
                {
                    return ParserExceptionType::ConflictingClassModifiers;
                }
                break;

            case ModifierDeclaration::Field:
                if (flags.contains(FlagType::Final) && flags.contains(FlagType::Volatile))
                {
                    return ParserExceptionType::ConflictingFieldModifiers;
                }
                break;

            case ModifierDeclaration::Method:
                if ((flags.contains(FlagType::Abstract)
                        && (flags.contains(FlagType::Private) || flags.contains(FlagType::Static)
                            || flags.contains(FlagType::Final) || flags.contains(FlagType::Synchronized)
                            || flags.contains(FlagType::Native) || flags.contains(FlagType::Strictfp)
                            || flags.contains(FlagType::Default)))
                    || (flags.contains(FlagType::Native) && flags.contains(FlagType::Strictfp))
                    || (flags.contains(FlagType::Default)
                        && (flags.contains(FlagType::Static) || flags.contains(FlagType::Private))))
                {
                    return ParserExceptionType::ConflictingMethodModifiers;
                }
                break;

            case ModifierDeclaration::Constructor:
                break;
            }
            return std::nullopt;
        };

        std::optional<ModifierConflict> accessConflict;
        std::optional<ModifierConflict> specificConflict;
        for (const auto& modifier : modifiers)
        {
            if (!modifier.flagType)
            {
                continue;
            }
            if (!flags.insert(*modifier.flagType).second)
            {
                return ModifierConflict{ParserExceptionType::DuplicateModifier, modifier.range};
            }
            if (!accessConflict
                && flags.count(FlagType::Public) + flags.count(FlagType::Protected) + flags.count(FlagType::Private) > 1)
            {
                accessConflict = ModifierConflict{ParserExceptionType::ConflictingAccessModifiers, modifier.range};
            }
            if (!specificConflict)
            {
                if (const auto exception = declarationConflict())
                {
                    specificConflict = ModifierConflict{*exception, modifier.range};
                }
            }
        }
        return accessConflict ? accessConflict : specificConflict;
    }
} // jcutils
