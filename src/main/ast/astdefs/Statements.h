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
    // An empty statement: ;
    struct EmptyStatement : Statement {};

    // A block containing statements in source order: { ... }.
    struct BlockStatement : Statement
    {
        std::vector<std::unique_ptr<Statement>> statements; // Statements in execution/source order.
    };

    // Wrap a local variable or local type declaration as a statement.
    struct DeclStatement : Statement
    {
        std::unique_ptr<Decl> declaration; // Local declaration wrapped by this statement.
    };

    // An expression used as a statement: foo(); or x++;
    struct ExprStatement : Statement
    {
        std::unique_ptr<Expr> expression; // Wrapped expression.
    };

    // Explicit constructor invocation: this(...) or super(...).
    struct ConstructorCallStatement : Statement
    {
        bool isSuper = false; // True for super(...); false for this(...).
        std::unique_ptr<Expr> qualifier; // Enclosing instance in outer.super(...); null when unqualified.
        std::vector<TypeNode> typeArguments; // Explicit type arguments at this use site.
        std::vector<std::unique_ptr<Expr>> arguments; // Arguments in source order.
    };

    // Return from a method or lambda: return [value];
    struct ReturnStatement : Statement
    {
        std::unique_ptr<Expr> value; // Optional return expression; null for return;
    };

    // if (condition) thenBranch [else elseBranch].
    struct IfStatement : Statement
    {
        std::unique_ptr<Expr> condition; // Condition expression.

        std::unique_ptr<Statement> thenBranch; // Statement executed when condition is true.
        std::unique_ptr<Statement> elseBranch; // Optional else statement; null when absent.
    };

    // while (condition) body.
    struct WhileStatement : Statement
    {
        std::unique_ptr<Expr> condition; // Condition expression.
        std::unique_ptr<Statement> body; // Body of this construct.
    };

    // do body while (condition);
    struct DoStatement : Statement
    {
        std::unique_ptr<Statement> body; // Body of this construct.
        std::unique_ptr<Expr> condition; // Condition expression.
    };

    // for (initializers; condition; updates) body.
    struct ForStatement : Statement
    {
        std::optional<VariableDecl> initializerDecl; // Variable initializer; mutually exclusive with initializerExprs.
        std::vector<std::unique_ptr<Expr>> initializerExprs; // Expression initializers; empty when initializerDecl is set.
        std::unique_ptr<Expr> condition; // Loop condition; null means omitted, as in for (;;).
        std::vector<std::unique_ptr<Expr>> updateExprs; // Expressions executed after each iteration.
        std::unique_ptr<Statement> body; // Body of this construct.
    };

    // for (Type name : iterableExpr) body.
    struct EnhancedForStatement : Statement
    {
        VariableDecl variableDecl; // Iteration variable; must contain exactly one declarator.
        std::unique_ptr<Expr> iterableExpr; // Array or Iterable expression being iterated.
        std::unique_ptr<Statement> body; // Body of this construct.
    };

    // A labeled statement: label: statement.
    struct LabeledStatement : Statement
    {
        std::u16string label; // Target or declared label.
        std::unique_ptr<Statement> statement; // Statement associated with the label.
    };

    // break; or break label;
    struct BreakStatement : Statement
    {
        std::optional<std::u16string> label; // Optional target label; absent for an unlabeled jump.
    };

    // continue; or continue label;
    struct ContinueStatement : Statement
    {
        std::optional<std::u16string> label; // Optional target label; absent for an unlabeled jump.
    };

    // Extension AllowGoto: goto label;
    struct GotoStatement : Statement
    {
        std::u16string label; // Target or declared label.
    };

    // assert condition [: message];
    struct AssertStatement : Statement
    {
        std::unique_ptr<Expr> condition; // Condition expression.
        std::unique_ptr<Expr> message; // Optional assertion message after :.
    };

    // synchronized (monitorExpr) { ... }.
    struct SynchronizedStatement : Statement
    {
        std::unique_ptr<Expr> monitorExpr; // Object whose monitor is acquired.
        std::unique_ptr<BlockStatement> body; // Body of this construct.
    };

    // One try resource; set either resourceDecl or referenceExpr.
    struct TryResource : ASTNode
    {
        std::optional<VariableDecl> resourceDecl; // New resource variable; absent when referenceExpr is set.
        std::unique_ptr<Expr> referenceExpr; // Existing resource reference; null when resourceDecl is set.
    };

    // catch (A | B name) { ... }.
    struct CatchClause : ASTNode
    {
        std::vector<ModifierFlag> modifiers; // Keyword modifiers and annotations in source order.
        std::vector<TypeNode> exceptionTypes; // Caught types: A and B in catch (A | B e).
        std::u16string name; // Declared or referenced name.
        std::unique_ptr<BlockStatement> body; // Body of this construct.
    };

    // try [(resources)] { ... } [catch ...] [finally ...].
    struct TryStatement : Statement
    {
        std::vector<TryResource> resources; // Try-with-resources entries in source order.
        std::unique_ptr<BlockStatement> tryBlock; // Main try body.
        std::vector<CatchClause> catches; // Catch clauses in source order.
        std::unique_ptr<BlockStatement> finallyBlock; // Optional finally body.
    };

    // Throw an exception: throw value;
    struct ThrowStatement : Statement
    {
        std::unique_ptr<Expr> value; // Exception expression to throw.
    };

    // A case/default label, optionally with a pattern and when guard.
    struct SwitchLabel : ASTNode
    {
        std::vector<std::unique_ptr<Expr>> caseExprs; // Case values, including NullExpr for case null.
        std::vector<std::unique_ptr<Pattern>> patterns; // Case matching patterns.
        bool hasDefault = false; // True for default, including case null, default.
        std::unique_ptr<Expr> guardExpr; // Optional condition after when.
    };

    // A colon statement group or an arrow rule; use only the matching body fields.
    struct SwitchCase : ASTNode
    {
        std::vector<SwitchLabel> labels; // Labels sharing this branch body.

        std::vector<std::unique_ptr<Statement>> statements; // Colon-group body; empty for arrow rules.
        std::unique_ptr<Expr> ruleExpr; // Expression after ->; null for colon groups and statement rules.
        std::unique_ptr<Statement> ruleStatement; // Block or throw after ->; null for expression rules and colon groups.
    };

    // A switch used as a statement: switch (selectorExpr) { ... }.
    struct SwitchStatement : Statement
    {
        std::unique_ptr<Expr> selectorExpr; // Expression inside switch (...).
        std::vector<SwitchCase> cases; // Switch branches in source order.
    };

    // Produce the value of a switch expression: yield value;
    struct YieldStatement : Statement
    {
        std::unique_ptr<Expr> value; // Value produced by the enclosing switch expression.
    };

} // javac

#endif //JAVACOMPILER_STATEMENTS_H
