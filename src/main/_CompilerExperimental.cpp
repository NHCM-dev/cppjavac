//
// Created by NHCM on 2026/11/26.
//

#include "Compiler.h"
#include "lexer/Lexer.h"
#include "tool/EncodingTool.h"
#include "tool/FileIoTool.h"
#include "util/ExceptionUtil.h"
#include "util/PrintUtil.h"

namespace javac
{
    namespace
    {
        struct AstTree
        {
            std::string label;
            std::vector<AstTree> children;

            void add(std::string label)
            {
                children.push_back({std::move(label), {}});
            }
        };

        void renderChildren(const AstTree& tree, const std::string& indent, std::string& output)
        {
            for (std::size_t i = 0; i < tree.children.size(); ++i)
            {
                const auto& child = tree.children[i];
                const bool last = i + 1 == tree.children.size();
                output += indent;
                output += last ? "└── " : "├── ";
                output += child.label + '\n';
                renderChildren(child, indent + (last ? "    " : "│   "), output);
            }
        }

        std::string arraySuffix(const std::vector<ArrayDimension>& dimensions)
        {
            std::string result;
            for (const auto& dimension : dimensions)
            {
                for (const auto& annotation : dimension.annotations)
                {
                    result += " @" + EncodingTool::utf16ToUtf8(annotation.name) + ' ';
                }
                result += "[]";
            }
            return result;
        }

        std::string typeName(const TypeNode& type)
        {
            if (type.elementType)
            {
                return typeName(*type.elementType) + arraySuffix(type.dimensions);
            }

            std::string result;
            for (const auto& annotation : type.annotations)
            {
                result += '@' + EncodingTool::utf16ToUtf8(annotation.name) + ' ';
            }
            if (type.primitiveType)
            {
                constexpr std::string_view names[] = {"boolean", "byte", "short", "char", "int", "long", "float", "double"};
                result += names[static_cast<std::size_t>(*type.primitiveType)];
                return result;
            }
            if (type.objectType == ObjectType::Void || type.objectType == ObjectType::Inferred)
            {
                result += type.objectType == ObjectType::Void ? "void" : "var";
                return result;
            }
            for (const auto& part : type.nameParts)
            {
                if (&part != &type.nameParts.front())
                {
                    result += '.';
                }
                for (const auto& annotation : part.annotations)
                {
                    result += '@' + EncodingTool::utf16ToUtf8(annotation.name) + ' ';
                }
                result += EncodingTool::utf16ToUtf8(part.name);
                if (part.diamond || !part.typeArguments.empty())
                {
                    result += '<';
                    for (const auto& argument : part.typeArguments)
                    {
                        if (&argument != &part.typeArguments.front())
                        {
                            result += ", ";
                        }
                        for (const auto& annotation : argument.annotations)
                        {
                            result += '@' + EncodingTool::utf16ToUtf8(annotation.name) + ' ';
                        }
                        switch (argument.genericType)
                        {
                            case GenericType::Exact: break;
                            case GenericType::Unbounded: result += '?'; break;
                            case GenericType::Extends: result += "? extends "; break;
                            case GenericType::Super: result += "? super "; break;
                        }
                        if (argument.type)
                        {
                            result += typeName(*argument.type);
                        }
                    }
                    result += '>';
                }
            }
            return result.empty() ? "<unknown type>" : result;
        }

        void addModifiers(AstTree& tree, const std::vector<ModifierFlag>& modifiers)
        {
            constexpr std::string_view names[] = {"public", "protected", "private", "abstract", "static", "final",
                "transient", "volatile", "synchronized", "native", "strictfp", "default", "sealed", "non-sealed"};
            for (const auto& modifier : modifiers)
            {
                if (modifier.flagType)
                {
                    tree.add("Modifier  " + std::string(names[static_cast<std::size_t>(*modifier.flagType)]));
                }
                if (modifier.annotation)
                {
                    tree.add("Annotation  @" + EncodingTool::utf16ToUtf8(modifier.annotation->name));
                }
            }
        }

        void addTypeParameters(AstTree& tree, const std::vector<GenericParameter>& parameters)
        {
            for (const auto& parameter : parameters)
            {
                AstTree node{"Type parameter  " + EncodingTool::utf16ToUtf8(parameter.name), {}};
                for (const auto& bound : parameter.bounds)
                {
                    node.add("Bound  " + typeName(bound));
                }
                tree.children.push_back(std::move(node));
            }
        }

        void addCallableChildren(AstTree& tree, const std::optional<ReceiverParameter>& receiver,
            const std::vector<std::unique_ptr<ParameterDecl>>& parameters, const std::vector<TypeNode>& thrownTypes,
            const BlockStatement* body)
        {
            if (receiver)
            {
                const std::string qualifier = receiver->qualifier ? EncodingTool::utf16ToUtf8(*receiver->qualifier) + '.' : "";
                tree.add("Receiver  " + typeName(receiver->type) + ' ' + qualifier + "this");
            }
            for (const auto& parameter : parameters)
            {
                AstTree node{"Parameter  " + EncodingTool::utf16ToUtf8(parameter->name), {}};
                addModifiers(node, parameter->modifiers);
                if (parameter->type)
                {
                    node.add("Type  " + typeName(*parameter->type) + arraySuffix(parameter->dimensions)
                        + (parameter->varargs ? "..." : ""));
                }
                tree.children.push_back(std::move(node));
            }
            for (const auto& type : thrownTypes)
            {
                tree.add("Throws  " + typeName(type));
            }
            if (body)
            {
                tree.add("Block  (" + std::to_string(body->statements.size()) + " statements)");
            }
        }

        AstTree declarationTree(const Decl& decl)
        {
            if (const auto* cls = dynamic_cast<const ClassDecl*>(&decl))
            {
                AstTree tree{"Class  " + EncodingTool::utf16ToUtf8(cls->name), {}};
                addModifiers(tree, cls->modifiers);
                addTypeParameters(tree, cls->typeParameters);
                if (cls->superClass)
                {
                    tree.add("Extends  " + typeName(*cls->superClass));
                }
                for (const auto& type : cls->interfaces)
                {
                    tree.add("Implements  " + typeName(type));
                }
                for (const auto& type : cls->permittedTypes)
                {
                    tree.add("Permits  " + typeName(type));
                }
                for (const auto& member : cls->members)
                {
                    tree.children.push_back(declarationTree(*member));
                }
                return tree;
            }
            if (const auto* field = dynamic_cast<const FieldDecl*>(&decl))
            {
                AstTree tree{"Field declaration", {}};
                addModifiers(tree, field->modifiers);
                tree.add("Type  " + typeName(field->type));
                for (const auto& variable : field->declarators)
                {
                    tree.add("Variable  " + EncodingTool::utf16ToUtf8(variable.name) + arraySuffix(variable.dimensions));
                }
                return tree;
            }
            if (const auto* method = dynamic_cast<const MethodDecl*>(&decl))
            {
                AstTree tree{"Method  " + EncodingTool::utf16ToUtf8(method->name), {}};
                addModifiers(tree, method->modifiers);
                addTypeParameters(tree, method->typeParameters);
                tree.add("Returns  " + typeName(method->returnType) + arraySuffix(method->returnDimensions));
                addCallableChildren(tree, method->receiverParam, method->parameters, method->thrownTypes, method->body.get());
                return tree;
            }
            if (const auto* ctor = dynamic_cast<const ConstructorDecl*>(&decl))
            {
                AstTree tree{"Constructor  " + EncodingTool::utf16ToUtf8(ctor->name), {}};
                addModifiers(tree, ctor->modifiers);
                addTypeParameters(tree, ctor->typeParameters);
                addCallableChildren(tree, ctor->receiverParam, ctor->parameters, ctor->thrownTypes, ctor->body.get());
                return tree;
            }
            if (const auto* init = dynamic_cast<const InitializerDecl*>(&decl))
            {
                AstTree tree{init->isStatic ? "Static initializer" : "Instance initializer", {}};
                if (init->body)
                {
                    tree.add("Block  (" + std::to_string(init->body->statements.size()) + " statements)");
                }
                return tree;
            }
            return {dynamic_cast<const EmptyDecl*>(&decl) ? "Empty declaration" : "Unsupported declaration", {}};
        }

        std::string renderAst(const Path& file, const CompilationUnit& unit)
        {
            AstTree tree{"Compilation unit  " + EncodingTool::utf16ToUtf8(file.filename().u16string()), {}};
            if (unit.packageDecl)
            {
                AstTree package{"Package  " + EncodingTool::utf16ToUtf8(unit.packageDecl->name), {}};
                for (const auto& annotation : unit.packageDecl->annotations)
                {
                    package.add("Annotation  @" + EncodingTool::utf16ToUtf8(annotation.name));
                }
                tree.children.push_back(std::move(package));
            }
            for (const auto& import : unit.importDecls)
            {
                tree.add("Import  " + EncodingTool::utf16ToUtf8(import.name));
            }
            if (unit.moduleDecl)
            {
                tree.add("Module  " + EncodingTool::utf16ToUtf8(unit.moduleDecl->name));
            }
            for (const auto& decl : unit.decls)
            {
                tree.children.push_back(declarationTree(*decl));
            }
            std::string output = tree.label + '\n';
            renderChildren(tree, {}, output);
            return output;
        }
    }

    bool Compiler::lexExperimental()
    {
        jcutils::printWarningCustom("This lex feature is currently experimental", LEXER);
        bool hasError = false;
        for (const auto& file : compileRtData->filesToCompile)
        {
            const std::string displayPath = file.string();

            if (verbose())
            {
                jcutils::printDebugCustom("Lexing file \"" + displayPath + "\"", LEXER);
            }

            std::u16string sourceDisplay;
            TokenStream tokenStream;
            try
            {
                sourceDisplay = EncodingTool::decodeToUtf16(FileIOTool::readUtf8(file), encoding());
                Lexer lexer(sourceDisplay, compilerConfig.extensions, compilerConfig.sourceLangLevel);
                tokenStream = lexer.lex(true, true);
            }
            catch (const std::invalid_argument& error)
            {
                noticeError(displayPath + ": " + error.what(), LEXER);
                hasError = true;
                continue;
            }
            catch (const std::ios_base::failure& error)
            {
                noticeError(error.what(), LEXER);
                hasError = true;
                continue;
            }
            hasError |= reportInvalidTokens(file, sourceDisplay, tokenStream);
            std::string tokenDisplay {};

            for (const auto& token : tokenStream.tokens)
            {
                if (token.tokenType != TokenType::Whitespace && token.tokenType != TokenType::NewLine)
                {
                    constexpr std::string_view typeNames[] = {
                        "EndOfFile", "Invalid", "Identifier", "Keyword",
                        "IntegerLiteral", "FloatLiteral", "CharacterLiteral", "StringLiteral",
                        "Symbol", "LineComment", "BlockComment", "JavadocComment",
                        "NewLine", "Whitespace", "BooleanLiteral", "NullLiteral"
                    };

                    std::string content;
                    for (const char16_t ch : token.content)
                    {
                        switch (ch)
                        {
                            case u'\n': content += "\\n"; break;
                            case u'\r': content += "\\r"; break;
                            case u'\t': content += "\\t"; break;
                            case u'"': content += "\\\""; break;
                            case u'\\': content += "\\\\"; break;
                            default:
                                if (ch >= u' ' && ch <= u'~')
                                {
                                    content += static_cast<char>(ch);
                                }
                                else
                                {
                                    char escape[7];
                                    std::snprintf(escape, sizeof(escape), "\\u%04X", static_cast<unsigned>(ch));
                                    content += escape;
                                }
                        }
                    }
                    const std::string position = sourceLocationToString(token.range.begin);
                    std::string output = position + ' ';
                    output += typeNames[static_cast<std::size_t>(token.tokenType)];
                    output += " \"" + content + '"';
                    if (token.invalidReason)
                    {
                        output += " (";
                        output += invalidTokenStr(*token.invalidReason);
                        output += ')';
                    }
                    tokenDisplay += output + "\n";
                }
            }
            sourceDisplay += u"\n\n===Lexer tokens===\n\n";
            sourceDisplay += EncodingTool::utf8ToUtf16(tokenDisplay);

            putFileToWrite(getWritePath(file, "javal"), std::vector(
            reinterpret_cast<const std::byte*>(sourceDisplay.data()),
            reinterpret_cast<const std::byte*>(sourceDisplay.data() + sourceDisplay.size())
            ));
        }

        return !hasError;
    }

    bool Compiler::reportParsingErrorExperimental(const Path& file, std::u16string_view source,
        const ParserResult& parserResult)
    {
        for (const auto& error : parserResult.exceptions)
        {
            std::string message = error.what();
            if (message.empty())
            {
                switch (error.exception)
                {
                    case ParserExceptionType::ExpectSymbol: message = "Expected a symbol"; break;
                    case ParserExceptionType::ExpectKeyword: message = "Expected a keyword"; break;
                    case ParserExceptionType::ExpectIdentifier: message = "Expected an identifier"; break;
                    case ParserExceptionType::InvalidClassMember: message = "Invalid class member declaration"; break;
                    case ParserExceptionType::ConstructorNameMismatch: message = "Constructor name must match its class"; break;
                    case ParserExceptionType::AbstractMethodInNonAbstractClass: message = "Abstract method requires an abstract class"; break;
                    case ParserExceptionType::UnsupportedFieldInitializer: message = "Field initializer parsing is not implemented"; break;
                    case ParserExceptionType::UnsupportedMemberTypeDeclaration: message = "Parsing this nested type is not implemented"; break;
                    case ParserExceptionType::UnsupportedElementExpression: message = "Annotation expression parsing is not implemented"; break;
                    default: message = "Parsing error (code " + std::to_string(static_cast<unsigned>(error.exception)) + ')';
                }
            }
            jcutils::printDiagnostic(jcutils::DiagnosticSeverity::Error, file, message, error.range, source, PARSER);
            compileRtData->errors.push_back(std::move(message));
        }
        return !parserResult.exceptions.empty();
    }

    bool Compiler::parseExperimental()
    {
        jcutils::printWarningCustom("This parse feature is currently experimental", PARSER);
        bool hasError = !lex();
        hasError |= !parse();
        for (const auto& [file, result] : compileRtData->parserResults)
        {
            if (!result.exceptions.empty())
            {
                continue;
            }
            const std::string output = renderAst(file, result.compilationUnit);
            const auto* bytes = reinterpret_cast<const std::byte*>(output.data());
            const Path destination = getWritePath(file, "ast");
            if (!putFileToWrite(destination, ByteArray(bytes, bytes + output.size())))
            {
                noticeError("Duplicate AST output path: " + destination.string(), PARSER);
                hasError = true;
            }
        }
        return !hasError;
    }
}
