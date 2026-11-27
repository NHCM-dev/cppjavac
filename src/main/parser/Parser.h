//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_PARSER_H
#define JAVACOMPILER_PARSER_H

#include <vector>

#include "ParserUtil.h"
#include "ast/ASTDef.h"
#include "main/parser/ast/astdefs/Declarations.h"
#include "main/parser/ast/astdefs/Expressions.h"
#include "main/core/def/Extension.h"
#include "main/lexer/LexerDef.h"

namespace javac
{
    struct ParserResult
    {
        CompilationUnit compilationUnit;
        std::vector<parser_exception> exceptions;
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
        std::optional<std::pair<std::size_t, Token>> typeCloseRemainder;






        // === basic tools ===

        // Returns the reference of token stream
        [[nodiscard]]
        const std::vector<Token>& tokens() const;

        // Return if an extension is enabled
        [[nodiscard]]
        bool extensionEnabled(JavacExtension extension) const;

        // Return true if current token is '@' and next token is not "interface" keyword
        [[nodiscard]]
        bool canBeAnnotation() const;

        // Return true if current lang level is equal ot greater than the target lang level
        [[nodiscard]]
        bool langLevelMeet(LangLevel targetLangLevel) const;

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






        // === Decl ===

        /**
         * Try parse a package declaration start from keyword "package"
         * @throw parser_exception if any exception occurred during parsing
         * @param annotations if there is any annotation need to be passed to the declaration
         * @return the constructed package declaration
         */
        PackageDecl parsePackageDecl(std::vector<Annotation> annotations = {});
        /**
         * Try parse an import declaration start from keyword "import"
         * @throw parser_exception if any exception occurred during parsing
         * @return the constructed import declaration
         */
        ImportDecl parseImportDecl();

        ClassDecl parseClassDecl(std::vector<ModifierFlag> modifiers = {},
            jcutils::ClassDeclContext context = jcutils::ClassDeclContext::TopLevel);
        /**
         * Try parse class members within class
         * @throws parser_exception if failed to parse any class member
         * @param className name of the enclosing class
         * @param classContext current class context provided to member parsing
         * @param methodContext context for methods declared in the class body
         * @return A pointer of the member declaration, Can be: ClassDecl, FieldDecl, MethodDecl, ConstructorDecl,
         * InitializerDecl, or return nullptr at the closing brace
         */
        Uptr<Decl> parseNormalClassMember(std::u16string_view className,
            jcutils::ClassDeclContext classContext, jcutils::MethodDeclContext methodContext,
            std::optional<ReceiverParameter> receiverParameter = std::nullopt);

        /**
         * Parse a field declaration after its type and first name have been consumed
         * This function will consume the semicolon at the end
         * @throws parser_exception if the field declaration is invalid or uses an unsupported initializer
         * @param modifiers parsed keyword modifiers and annotations
         * @param context context of the body containing the field declaration
         * @param type parsed type shared by all declarators
         * @param firstName token of the first field name
         * @return parsed field declaration
         */
        FieldDecl parseFieldDecl(std::vector<ModifierFlag> modifiers, jcutils::ClassDeclContext context,
            TypeNode type, const Token& firstName);

        /**
         * Parse a local variable declaration with one or more declarators
         * @throws parser_exception if failed to parse the variable declaration
         * @param modifiers already parsed modifiers and annotations, or empty to parse them from the input
         * @param consumeSemiColon whether to consume the semicolon at the end
         * @param context syntactic context of the declaration
         * @return parsed variable declaration
         */
        VariableDecl parseVariableDecl(std::vector<ModifierFlag> modifiers = {},
            bool consumeSemiColon = true,
            jcutils::VariableDeclContext context = jcutils::VariableDeclContext::Local);

        // Method declaration
        Uptr<InitializerDecl> parseInitializerDecl();
        MethodDecl parseMethodDecl(std::vector<ModifierFlag> modifiers, jcutils::MethodDeclContext context,
            std::vector<GenericParameter> typeParameters, TypeNode returnType, const Token& returnTypeStart,
            const Token& name, std::optional<ReceiverParameter> receiverParameter = std::nullopt);
        ConstructorDecl parseConstructorDecl(std::vector<ModifierFlag> modifiers, jcutils::ClassDeclContext context,
            std::vector<GenericParameter> typeParameters, const Token& name,
            std::optional<ReceiverParameter> receiverParameter = std::nullopt);
        /**
         * Parse parameters declaration in the parenthesis of a method declaration
         * Expect that a '(' token has been consumed. This function will consume a ')' token at the end
         * @throws parser_exception if failed to parse parameters
         * @return parsed method parameters
         */
        std::vector<Uptr<ParameterDecl>> parseMethodParameters();

        // ======







        // === Expr ===

        /**
         * Try parse annotations, will consume an '@' symbol token
         * @return a vector of parsed annotations or empty if nothing parsed
         */
        std::vector<Annotation> parseAnnotations();
        Annotation parseAnnotation();

        // Element value (value in annotation value or method default value)
        /**
         * Parse value for both the value fit in annotation declaration and method default value declaration
         * @return parsed expression for the value
         */
        Uptr<Expr> parseElementValue();
        /**
         * Parse an array initializer for annotation value, expect a '{' is consumed already
         * @return pointer of an ArrayInitializerExpr
         */
        Uptr<Expr> parseElementArray();
        Uptr<Expr> parseElementExpr();

        /**
         * Treat current token as basic literal (string, int, etc) and try to parse it
         * @throw parser_exception if rhw token is not a valid basic literal
         * @return parsed basic literal
         */
        Uptr<Expr> parseBasicLiteral();
         /**
         * Try to parse until construct a valid class literal (XXX.class) and return it
         * @throw parser_exception if there is no class literal to parse start from now
         * @return parsed class literal
         */
        Uptr<ClassLiteralExpr> parseClassLiteral();

        /**
         * A value which can be confirmed during compile time
         * Such as number, enum, static final value, Java17+: string, null.
         * Does not include method call, annotation etc
         * @return parsed expression
         */
        Uptr<Expr> parseConstantValue();
        /**
        * A value represented by expression, literally can be anything
        * @return parsed expression
        */
        Uptr<Expr> parseValue();
        /**
         * Parse one statement expression: an assignment, an increment or decrement,
         * a method invocation, or a class instance creation expression.
         * Leave the following semicolon, comma or right parenthesis for the caller.
         * @throws parser_exception if the expression is invalid or cannot be used as a statement
         * @return parsed expression
         */
        Uptr<Expr> parseStatementExpression();
        Uptr<Expr> parseVariableInitializer();

        // ======







        // === Statement ===

        Uptr<ExprStatement> parseExprStatement();
        Uptr<DeclStatement> parseVarDeclStatement();
        Uptr<ConstructorCallStatement> parseConstructorCallStatement();

        /**
         * Parse code block start with '{' and end with '}'
         * @throw parser_exception if the block is invalid
         * @return parsed block statement
         */
        Uptr<BlockStatement> parseBlock();
        /**
       * Parse a single code statement
       * @throw parser_exception if the statement is invalid
       * @return parsed statement
       */
        Uptr<Statement> parseStatement(jcutils::StatementContext context);

        Uptr<ReturnStatement> parseReturnStatement();

        Uptr<IfStatement> parseIfStatement();
        Uptr<WhileStatement> parseWhileStatement();
        Uptr<DoWhileStatement> parseDoWhileStatement();
        Uptr<ForStatement> parseForStatement();
        Uptr<EnhancedForStatement> parseEnhancedForStatement();

        Uptr<SwitchStatement> parseSwitchStatement();

        Uptr<ThrowStatement> parseThrowStatement();
        Uptr<LabeledStatement> parseLabeledStatement();
        Uptr<ContinueStatement> parseContinueStatement();
        // This one is for extension
        Uptr<GotoStatement> parseGotoStatement();
        Uptr<AssertStatement> parseAssertStatement();
        Uptr<BreakStatement> parseBreakStatement();

        Uptr<SynchronizedStatement> parseSynchronizedStatement();

        Uptr<TryStatement> parseTryStatement();
        TryResource parseTryResource();

        Uptr<YieldStatement> parseYieldStatement();

        // ======







        // === Other syntax and compilation units ===

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

        std::vector<ModifierFlag> parseModifierFlags();
        AnnotationArg parseAnnotationArg();
        SwitchLabel parseSwitchLabel();

        /**
         * Parse generic parameters for class declaration or method declaration
         * Like "<T extends X>"
         * @return parsed GenericParameters
         */
        std::vector<GenericParameter> parseGenericParameters();

        CatchClause parseCatchClause();





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

        ParserResult parse();
    };

} // javac

#endif //JAVACOMPILER_PARSER_H
