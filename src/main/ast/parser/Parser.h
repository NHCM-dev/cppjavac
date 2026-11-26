//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_PARSER_H
#define JAVACOMPILER_PARSER_H

#include "ParserUtil.h"
#include <vector>

#include "main/ast/astdefs/Declarations.h"
#include "main/ast/astdefs/Expressions.h"
#include "main/core/def/Extension.h"
#include "main/lexer/LexerDef.h"

namespace javac
{
    enum class ParserExceptionType : uint8_t
    {
        ExpectSymbol,
        ExpectKeyword,
        ExpectIdentifier,
        ExpectClassLiteral,

        MultipleExtends,
        MultipleImplements,
        MultiplePermits,

        NeedBraceForClassDeclaration,

        NotLiteral,

        InvalidNumberLiteral,
        OutOfRangeNumberLiteral,

        KeywordInTypeSpecification,
    };

    class parser_exception : public std::logic_error
    {
    public:
        const ParserExceptionType exception;
        const std::optional<Token> actualToken = std::nullopt;

        parser_exception(ParserExceptionType exception)
            : logic_error({}), exception(exception)
        {

        }

        parser_exception(ParserExceptionType exception, Token actualToken)
            : logic_error({}), exception(exception), actualToken(actualToken)
        {

        }

        parser_exception(ParserExceptionType exception, Token actualToken, const std::string& cause)
            : logic_error(cause), exception(exception), actualToken(actualToken)
        {

        }
    };

    /**
     * Class that parse a valid stream of tokens and generate AST tree
     */
    class Parser
    {
        const Path filePath;
        TokenStream inputStream;
        const std::set<JavacExtension> extensions;
        const LangLevel sourceLangLevel;

        CompilationUnit compilationUnit = {};


        static CompilationUnitType decideType(const Path& filePath);

        // index which point to current unread-unconsumed token
        size_t tokenPointer = 0;

        // === basic tools ===

        // Returns the reference of token stream
        [[nodiscard]]
        const std::vector<Token>& tokens() const;

        /**
         * Read token at X offset
         * Invoke without arg to get current token
         * @param offset offset of tokenPointer
         * @return token read
         */
        [[nodiscard]]
        const Token& peek(size_t offset = 0) const;
        // consume current token and make pointer+1
        const Token& advance();
        // Return true if current pointer reached EOF
        [[nodiscard]]
        bool isAtEnd() const;

        // Return true if current token match the given symbol
        [[nodiscard]]
        bool checkSymbol(SymbolType type) const;
        // If current token match the given symbol, return true and advance, otherwise return false
        bool matchSymbol(SymbolType type);

        /**
         * If current token match the given symbol, return the current token and advance, otherwise throw exception
         * @throw parser_exception if the current token does not match the given type
         * @param type symbol type that needs to be matched
         * @return expected token
         */
        const Token& expectSymbol(SymbolType type);

        // Return true if current token is a keyword
        [[nodiscard]]
        bool checkKeyword() const;
        // Return true if current token match the given keyword
        [[nodiscard]]
        bool checkKeyword(const JavaKeyword& keyword) const;
        // If current token match the given keyword, return true and advance, otherwise return false
        bool matchKeyword(const JavaKeyword& keyword);
        /**
         * If current token match the given keyword, return the current token and advance, otherwise throw exception
         * @throw parser_exception if the current token does not match the given keyword
         * @param keyword keyword that needs to be matched
         * @return expected token
         */
        const Token& expectKeyword(const JavaKeyword& keyword);

        // Return true if current token is an identifier
        [[nodiscard]]
        bool checkIdentifier() const;
        // If current token is an identifier, return true and advance, otherwise return false
        bool matchIdentifier();
        /**
          * If current token is an identifier, return the current token and advance, otherwise throw exception
          * @throw parser_exception if the current token is not an identifier
          * @return expected token
          */
        const Token& expectIdentifier();

        // ======

        /**
         * Parse a TypeNode if you are sure that from this token it is going to be a type specification
         * Will auto proces annotations at the front
         * @return TypeNode parsed
         */
        TypeNode parseTypeNode();

        /**
         * Parse a vector of ArrayDimension for type specification
         * Annotations are automatically process, include the prefix one
         * @return parsed ArrayDimensions or empty if nothing parsed
         */
        std::vector<ArrayDimension> parseArrayDimension();

        // Parse TypeArg defined within <...>, Like "String" in "List<String>"
        std::vector<TypeArg> parseTypeArgs();

        /**
         * Try parse a package declaration start from keyword "package"
         * @throw parser_exception if any exception occurred during parsing
         * @param annotations if there is any annotation need to be passed to the declaration
         * @return the constructed package declaration
         */
        PackageDecl parsePackageDecl(std::vector<Annotation> annotations = {});
        ClassDecl parseClassDecl(std::vector<ModifierFlag> modifiers = {});
        MethodDecl parseMethodDecl();

        /**
         * Try parse annotations, will consume an '@' symbol token
         * @return a vector of parsed annotations or empty if nothing parsed
         */
        std::vector<Annotation> parseAnnotations();
        Annotation parseAnnotation();
        AnnotationArg parseAnnotationArg();
        std::unique_ptr<Expr> parseAnnotationValue();
        std::unique_ptr<Expr> parseAnnotationArray();
        std::unique_ptr<Expr> parseAnnotationExpr();

        std::vector<GenericParameter> parseGenericParameters();

        /**
         * Treat next token as basic literal (string, int, etc) and try to parse it
         * @throw parser_exception if next token is not a valid basic literal
         * @return parsed basic literal
         */
        std::unique_ptr<Expr> parseBasicLiteral();
         /**
         * Try to parse until construct a valid class literal (XXX.class) and return it
         * @throw parser_exception if there is no class literal to parse start from now
         * @return parsed class literal
         */
        std::unique_ptr<ClassLiteralExpr> parseClassLiteral();

        void parseOrdinary();
        void parseModular();

    public:
        /**
         * @throws std::invalid_argument if the given token streams contains invalid tokens
         * @param inputStream Token streams which contains no invalid tokens
         * @param filePath path of source file
         */
        Parser(TokenStream inputStream, Path filePath,
            std::set<JavacExtension> extensions = {}, LangLevel sourceLangLevel = LangLevel::Eight);

        CompilationUnit parse();
    };

} // javac

#endif //JAVACOMPILER_PARSER_H
