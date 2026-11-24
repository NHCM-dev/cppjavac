//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_STATEMENTS_H
#define JAVACOMPILER_STATEMENTS_H
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "main/ast/ASTDef.h"

namespace javac
{
    struct EmptyStatement : Statement {};

    struct BlockStatement : Statement
    {
        std::vector<std::unique_ptr<Statement>> statements;
    };

    struct DeclStatement : Statement
    {
        std::unique_ptr<Decl> declaration;
    };

    struct ExprStatement : Statement
    {
        std::unique_ptr<Expr> expression;
    };

    struct ConstructorInvocationStatement : Statement
    {
        bool isSuper = false; // super(...) or this(...)
        std::unique_ptr<Expr> qualifier;
        std::vector<TypeExpr> typeArguments;
        std::vector<std::unique_ptr<Expr>> arguments;
    };

    struct ReturnStatement : Statement
    {
        std::unique_ptr<Expr> value; // Empty for return;
    };

    struct IfStatement : Statement
    {
        std::unique_ptr<Expr> condition;

        std::unique_ptr<Statement> thenBranch;
        std::unique_ptr<Statement> elseBranch;
    };

    struct WhileStatement : Statement
    {
        std::unique_ptr<Expr> condition;
        std::unique_ptr<Statement> body;
    };

    struct DoStatement : Statement
    {
        std::unique_ptr<Statement> body;
        std::unique_ptr<Expr> condition;
    };

    struct ForStatement : Statement
    {
        std::optional<VariableDecl> initializerDecl;
        std::vector<std::unique_ptr<Expr>> initializerExpressions;
        std::unique_ptr<Expr> condition;
        std::vector<std::unique_ptr<Expr>> updates;
        std::unique_ptr<Statement> body;
    };

    struct EnhancedForStatement : Statement
    {
        VariableDecl variable;
        std::unique_ptr<Expr> iterable;
        std::unique_ptr<Statement> body;
    };

    struct LabeledStatement : Statement
    {
        std::u16string label;
        std::unique_ptr<Statement> statement;
    };

    struct BreakStatement : Statement
    {
        std::optional<std::u16string> label;
    };

    struct ContinueStatement : Statement
    {
        std::optional<std::u16string> label;
    };

    // AllowGoto
    struct GotoStatement : Statement
    {
        std::u16string label;
    };

    struct AssertStatement : Statement
    {
        std::unique_ptr<Expr> condition;
        std::unique_ptr<Expr> message;
    };

    struct SynchronizedStatement : Statement
    {
        std::unique_ptr<Expr> monitor;
        std::unique_ptr<BlockStatement> body;
    };

    struct TryResource : ASTNode
    {
        std::optional<VariableDecl> declaration;
        std::unique_ptr<Expr> reference;
    };

    struct CatchClause : ASTNode
    {
        std::vector<AdditionalFlags> modifiers;
        std::vector<TypeExpr> exceptionTypes; // A | B
        std::u16string name;
        std::unique_ptr<BlockStatement> body;
    };

    struct TryStatement : Statement
    {
        std::vector<TryResource> resources;
        std::unique_ptr<BlockStatement> block;
        std::vector<CatchClause> catches;
        std::unique_ptr<BlockStatement> finallyBlock;
    };

    struct ThrowStatement : Statement
    {
        std::unique_ptr<Expr> value;
    };

    struct SwitchLabel : ASTNode
    {
        std::vector<std::unique_ptr<Expr>> expressions;
        std::vector<std::unique_ptr<Pattern>> patterns;
        bool hasDefault = false; // Also set for case null, default
        std::unique_ptr<Expr> guard; // when
    };

    struct SwitchCase : ASTNode
    {
        std::vector<SwitchLabel> labels;

        std::vector<std::unique_ptr<Statement>> statements; // Colon group
        std::unique_ptr<Expr> ruleExpression;
        std::unique_ptr<Statement> ruleStatement; // Arrow rule with a block or throw
    };

    struct SwitchStatement : Statement
    {
        std::unique_ptr<Expr> selector;
        std::vector<SwitchCase> cases;
    };

    struct YieldStatement : Statement
    {
        std::unique_ptr<Expr> value;
    };

} // javac

#endif //JAVACOMPILER_STATEMENTS_H
