//
// Created by NHCM on 2026/11/23.
//

#ifndef JAVACOMPILER_STATEMENTS_H
#define JAVACOMPILER_STATEMENTS_H
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "main/parser/ast/ASTDef.h"

namespace javac
{
    // An empty statement: ;
    struct EmptyStatement : Statement {};

    // A block containing statements in source order: { ... }.
    struct BlockStatement : Statement
    {
        std::vector<Uptr<Statement>> statements; // Statements in execution/source order.
    };

    // Wrap a local variable or local type declaration as a statement.
    struct DeclStatement : Statement
    {
        Uptr<Decl> declaration; // Local declaration wrapped by this statement.
    };

    // An expression used as a statement: foo(); or x++;
    struct ExprStatement : Statement
    {
        Uptr<Expr> expression; // Wrapped expression.
    };

    // Explicit constructor invocation: this(...) or super(...).
    struct ConstructorCallStatement : Statement
    {
        bool isSuper = false; // True for super(...); false for this(...).
        Uptr<Expr> qualifier; // Enclosing instance in outer.super(...); null when unqualified.
        std::vector<TypeNode> typeArguments; // Explicit type arguments at this use site.
        std::vector<Uptr<Expr>> arguments; // Arguments in source order.
    };

    // Return from a method or lambda: return [value];
    struct ReturnStatement : Statement
    {
        Uptr<Expr> value; // Optional return expression; null for return;
    };

    // if (condition) thenBranch [else elseBranch].
    struct IfStatement : Statement
    {
        Uptr<Expr> condition; // Condition expression.

        Uptr<Statement> thenBranch; // Statement executed when condition is true.
        Uptr<Statement> elseBranch; // Optional else statement; null when absent.
    };

    // while (condition) body.
    struct WhileStatement : Statement
    {
        Uptr<Expr> condition; // Condition expression.
        Uptr<Statement> body; // Body of this construct.
    };

    // do body while (condition);
    struct DoWhileStatement : Statement
    {
        Uptr<Statement> body; // Body of this construct.
        Uptr<Expr> condition; // Condition expression.
    };

    // for (initializers; condition; updates) body.
    struct ForStatement : Statement
    {
        std::optional<VariableDecl> initializerDecl; // Variable initializer; mutually exclusive with initializerExprs.
        std::vector<Uptr<Expr>> initializerExprs; // Expression initializers; empty when initializerDecl is set.
        Uptr<Expr> condition; // Loop condition; null means omitted, as in for (;;).
        std::vector<Uptr<Expr>> updateExprs; // Expressions executed after each iteration.
        Uptr<Statement> body; // Body of this construct.
    };

    // for (Type name : iterableExpr) body.
    struct EnhancedForStatement : Statement
    {
        VariableDecl variableDecl; // Iteration variable; must contain exactly one declarator.
        Uptr<Expr> iterableExpr; // Array or Iterable expression being iterated.
        Uptr<Statement> body; // Body of this construct.
    };

    // A labeled statement: label: statement.
    struct LabeledStatement : Statement
    {
        std::u16string label; // Target or declared label.
        Uptr<Statement> statement; // Statement associated with the label.
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
        Uptr<Expr> condition; // Condition expression.
        Uptr<Expr> message; // Optional assertion message after :.
    };

    // synchronized (monitorExpr) { ... }.
    struct SynchronizedStatement : Statement
    {
        Uptr<Expr> monitorExpr; // Object whose monitor is acquired.
        Uptr<BlockStatement> body; // Body of this construct.
    };

    // One try resource; set either resourceDecl or referenceExpr.
    struct TryResource : ASTNode
    {
        std::optional<VariableDecl> resourceDecl; // New resource variable; absent when referenceExpr is set.
        Uptr<Expr> referenceExpr; // Existing resource reference; null when resourceDecl is set.
    };

    // catch (A | B name) { ... }.
    struct CatchClause : ASTNode
    {
        std::vector<ModifierFlag> modifiers; // Keyword modifiers and annotations in source order.
        std::vector<TypeNode> exceptionTypes; // Caught types: A and B in catch (A | B e).
        std::u16string name; // Declared or referenced name.
        Uptr<BlockStatement> body; // Body of this construct.
    };

    // try [(resources)] { ... } [catch ...] [finally ...].
    struct TryStatement : Statement
    {
        std::vector<TryResource> resources; // Try-with-resources entries in source order.
        Uptr<BlockStatement> tryBlock; // Main try body.
        std::vector<CatchClause> catches; // Catch clauses in source order.
        Uptr<BlockStatement> finallyBlock; // Optional finally body.
    };

    // Throw an exception: throw value;
    struct ThrowStatement : Statement
    {
        Uptr<Expr> value; // Exception expression to throw.
    };

    // A case/default label, optionally with a pattern and when guard.
    struct SwitchLabel : ASTNode
    {
        std::vector<Uptr<Expr>> caseExprs; // Case values, including NullExpr for case null.
        std::vector<Uptr<Pattern>> patterns; // Case matching patterns.
        bool hasDefault = false; // True for default, including case null, default.
        Uptr<Expr> guardExpr; // Optional condition after when.
    };

    // A colon statement group or an arrow rule; use only the matching body fields.
    struct SwitchCase : ASTNode
    {
        std::vector<SwitchLabel> labels; // Labels sharing this branch body.

        std::vector<Uptr<Statement>> statements; // Colon-group body; empty for arrow rules.
        Uptr<Expr> ruleExpr; // Expression after ->; null for colon groups and statement rules.
        Uptr<Statement> ruleStatement; // Block or throw after ->; null for expression rules and colon groups.
    };

    // A switch used as a statement: switch (selectorExpr) { ... }.
    struct SwitchStatement : Statement
    {
        Uptr<Expr> selectorExpr; // Expression inside switch (...).
        std::vector<SwitchCase> cases; // Switch branches in source order.
    };

    // Produce the value of a switch expression: yield value;
    struct YieldStatement : Statement
    {
        Uptr<Expr> value; // Value produced by the enclosing switch expression.
    };

} // javac

#endif //JAVACOMPILER_STATEMENTS_H
