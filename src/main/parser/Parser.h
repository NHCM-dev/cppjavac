//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_PARSER_H
#define JAVACOMPILER_PARSER_H

#include <vector>

#include "ast/ASTDef.h"
#include "main/parser/ast/astdefs/Declarations.h"
#include "main/parser/ast/astdefs/Expressions.h"
#include "main/core/def/Extension.h"
#include "main/lexer/LexerDef.h"

namespace javac
{
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
        std::optional<std::pair<std::size_t, Token>> typeCloseRemainder;

        // === basic tools ===

        // Returns the reference of token stream
        [[nodiscard]]
        const std::vector<Token>& tokens() const;

        // Return if an extension is enabled
        [[nodiscard]]
        bool extensionEnabled(JavacExtension extension) const;

        [[nodiscard]]
        bool canBeAnnotation() const;

        /**
         * Read token at X offset
         * Invoke without arg to get current token
         * @param offset offset of tokenPointer
         * @return token read
         */
        [[nodiscard]]
        const Token& peek(size_t offset = 0) const;
        // consume current token and make pointer+1
        // Returns the current token (not next)
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
        /**
         * Consume one closing angle bracket in a generic type.
         * If the current token starts with '>' and contains more characters,
         * leave the remaining symbol at the cursor.
         * @throw parser_exception if the current token cannot provide a closing angle bracket
         */
        void expectTypeClose();
        void errorProcessPlaceholder();

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
         * When varargs is supplied, consumes and records a trailing annotated or bare ellipsis.
         * @return TypeNode parsed
         */
        TypeNode parseTypeNode(std::vector<Annotation> annotations = {},
            std::optional<ArrayDimension>* varargs = nullptr);

        /**
         * Parse a vector of ArrayDimension for type specification
         * Annotations are automatically process, include the prefix one
         * @return parsed ArrayDimensions or empty if nothing parsed
         */
        std::vector<ArrayDimension> parseArrayDimension(bool allowAnnotation = true,
            std::optional<ArrayDimension>* varargs = nullptr);

        // Parse TypeArg defined within <...>, Like "String" in "List<String>"
        // It must be specific type or unbound, not for class generic parameter declaration
        // By invoking this method, a Left angle bracket should be consumed, and this method won't consume the Right angle bracket
        std::vector<TypeArg> parseTypeArgs();
        

        /**
         * Try parse a package declaration start from keyword "package"
         * @throw parser_exception if any exception occurred during parsing
         * @param annotations if there is any annotation need to be passed to the declaration
         * @return the constructed package declaration
         */
        PackageDecl parsePackageDecl(std::vector<Annotation> annotations = {});
        ClassDecl parseClassDecl(std::vector<ModifierFlag> modifiers = {});

        std::vector<ModifierFlag> parseModifierFlags();

        FieldDecl parseFieldDecl(std::vector<ModifierFlag> modifiers = {});


        // Method declaration
        MethodDecl parseMethodDecl(std::vector<ModifierFlag> modifiers = {},
         std::optional<ReceiverParameter> receiverParameter = std::nullopt);
        /**
         * Parse parameters declaration in the parenthesis of a method declaration
         * Expect that a '(' token has been consumed. This function will consume a ')' token at the end
         * @throws parser_exception if failed to parse parameters
         * @return parsed method parameters
         */
        std::vector<std::unique_ptr<ParameterDecl>> parseMethodParameters();
        BlockStatement parseMethodBody();

        // Annotation declaration
        /**
         * Try parse annotations, will consume an '@' symbol token
         * @return a vector of parsed annotations or empty if nothing parsed
         */
        std::vector<Annotation> parseAnnotations();
        Annotation parseAnnotation();
        AnnotationArg parseAnnotationArg();

        // Element value (value in annotation value or method default value)
        /**
         * Parse value for both the value fit in annotation declaration and method default value declaration
         * @return parsed expression for the value
         */
        std::unique_ptr<Expr> parseElementValue();
        /**
         * Parse an array initializer for annotation value, expect a '{' is consumed already
         * @return pointer of an ArrayInitializerExpr
         */
        std::unique_ptr<Expr> parseElementArray();
        std::unique_ptr<Expr> parseElementExpr();

        /**
         * Parse generic parameters for class declaration or method declaration
         * Like "<T extends X>"
         * @return parsed GenericParameters
         */
        std::vector<GenericParameter> parseGenericParameters();

        /**
         * Treat current token as basic literal (string, int, etc) and try to parse it
         * @throw parser_exception if rhw token is not a valid basic literal
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
