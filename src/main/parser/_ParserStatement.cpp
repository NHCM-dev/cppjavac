#include "Parser.h"

#include <utility>

#include "ParserException.h"

namespace javac
{
    Uptr<BlockStatement> Parser::parseBlock()
    {
        Uptr<BlockStatement> blockStatement = makeUniq<BlockStatement>();
        blockStatement->range = expectSymbol(SymbolType::LBrace).range;

        while (!isAtEnd() && !checkSymbol(SymbolType::RBrace))
        {
            blockStatement->statements.push_back(parseStatement(jcutils::StatementContext::Block));
        }

        blockStatement->range->end = expectSymbol(SymbolType::RBrace).range.end;
        return blockStatement;
    }

    Uptr<Statement> Parser::parseStatement([[maybe_unused]] jcutils::StatementContext context)
    {
        if (checkSymbol(SymbolType::LBrace))
        {
            return parseBlock();
        }

        if (checkSymbol(SymbolType::Semicolon))
        {
            auto statement = makeUniq<EmptyStatement>();
            statement->range = advance().range;
            return statement;
        }
        if (checkKeyword(THROW))            return parseThrowStatement();
        if (checkKeyword(RETURN))           return parseReturnStatement();
        if (checkKeyword(IF))               return parseIfStatement();
        if (checkKeyword(SWITCH))           return parseSwitchStatement();
        if (checkKeyword(CONTINUE))         return parseContinueStatement();
        if (checkKeyword(GOTO))             return parseGotoStatement();
        if (checkKeyword(ASSERT))           return parseAssertStatement();
        if (checkKeyword(BREAK))            return parseBreakStatement();
        if (checkKeyword(SYNCHRONIZED))     return parseSynchronizedStatement();
        if (checkKeyword(TRY))              return parseTryStatement();

        if (checkIdentifier() && peek(1).symbol == SymbolType::Colon)
        {
            return parseLabeledStatement();
        }

        std::vector<Annotation> pendingAnnotations = {};
        if (canBeAnnotation())
        {
            pendingAnnotations = parseAnnotations();
        }

        //TODO: Parse local declarations only in StatementContext::Block.
        //TODO: Implement rest of the statement parsing
        throw parser_exception(ParserExceptionType::UnsupportedStatement, peek());
    }

    Uptr<ReturnStatement> Parser::parseReturnStatement()
    {
        Uptr<ReturnStatement> statement = makeUniq<ReturnStatement>();

        statement->range = expectKeyword(RETURN).range;

        if (!checkSymbol(SymbolType::Semicolon))
        {
            statement->value = parseValue();
        }

        statement->range->end = expectSymbol(SymbolType::Semicolon).range.end;

        return statement;
    }

    Uptr<IfStatement> Parser::parseIfStatement()
    {
        Uptr<IfStatement> statement = makeUniq<IfStatement>();

        statement->range = expectKeyword(IF).range;

        expectSymbol(SymbolType::LParen);
        statement->condition = parseValue();
        expectSymbol(SymbolType::RParen);

        statement->thenBranch = parseStatement(jcutils::StatementContext::Embedded);
        if (matchKeyword(ELSE))
        {
            statement->elseBranch = parseStatement(jcutils::StatementContext::Embedded);
        }

        const Uptr<Statement>& lastBranch = statement->elseBranch
            ? statement->elseBranch
            : statement->thenBranch;
        statement->range->end = lastBranch->range->end;
        return statement;
    }

    Uptr<SwitchStatement> Parser::parseSwitchStatement()
    {
        Uptr<SwitchStatement> statement = makeUniq<SwitchStatement>();

        statement->range = expectKeyword(SWITCH).range;

        expectSymbol(SymbolType::LParen);
        statement->selectorExpr = parseValue();
        expectSymbol(SymbolType::RParen);

        expectSymbol(SymbolType::LBrace);
        const bool allowArrow = langLevelMeet(LangLevel::Fourteen);
        const auto atLabel = [this]
        {
            return checkKeyword(CASE) || checkKeyword(DEFAULT);
        };

        std::optional<SymbolType> switchSeparator;
        while (atLabel())
        {
            SwitchCase branch;
            do
            {
                branch.labels.push_back(parseSwitchLabel());
                const Token& separator = peek();
                if (!checkSymbol(SymbolType::Colon) && !(allowArrow && checkSymbol(SymbolType::Arrow)))
                {
                    throw parser_exception(allowArrow ? ParserExceptionType::ExpectColonOrArrow
                        : ParserExceptionType::ExpectColon, separator);
                }
                if (switchSeparator && switchSeparator != separator.symbol)
                {
                    throw parser_exception(ParserExceptionType::ArrowColonMixUseInSwitch, separator);
                }

                switchSeparator = separator.symbol;
                advance();
                if (!branch.range)
                {
                    branch.range = branch.labels.front().range;
                }
                branch.range->end = separator.range.end;
            } while (switchSeparator == SymbolType::Colon && atLabel());

            if (switchSeparator == SymbolType::Colon)
            {
                while (!isAtEnd() && !checkSymbol(SymbolType::RBrace) && !atLabel())
                {
                    auto child = parseStatement(jcutils::StatementContext::Block);
                    branch.range->end = child->range->end;
                    branch.statements.push_back(std::move(child));
                }
            }
            else if (checkSymbol(SymbolType::LBrace))
            {
                branch.ruleStatement = parseBlock();
                branch.range->end = branch.ruleStatement->range->end;
            }
            else if (checkKeyword(THROW))
            {
                branch.ruleStatement = parseThrowStatement();
                branch.range->end = branch.ruleStatement->range->end;
            }
            else
            {
                branch.ruleExpr = parseValue();
                branch.range->end = expectSymbol(SymbolType::Semicolon).range.end;
            }
            statement->cases.push_back(std::move(branch));
        }

        statement->range->end = expectSymbol(SymbolType::RBrace).range.end;
        return statement;
    }

    Uptr<ThrowStatement> Parser::parseThrowStatement()
    {
        auto statement = makeUniq<ThrowStatement>();

        statement->range = expectKeyword(THROW).range;
        statement->value = parseValue();
        statement->range->end = expectSymbol(SymbolType::Semicolon).range.end;

        return statement;
    }

    Uptr<LabeledStatement> Parser::parseLabeledStatement()
    {
        Uptr<LabeledStatement> statement = makeUniq<LabeledStatement>();

        const Token& label = expectIdentifier();
        statement->range = label.range;
        statement->label = label.content;

        expectSymbol(SymbolType::Colon);
        statement->statement = parseStatement(jcutils::StatementContext::Embedded);
        statement->range->end = statement->statement->range->end;

        return statement;
    }

    Uptr<ContinueStatement> Parser::parseContinueStatement()
    {
        Uptr<ContinueStatement> statement = makeUniq<ContinueStatement>();
        statement->range = expectKeyword(CONTINUE).range;

        if (checkIdentifier())
        {
            const Token& label = expectIdentifier();
            statement->label = label.content;
            statement->range->end = label.range.end;
        }

        statement->range->end = expectSymbol(SymbolType::Semicolon).range.end;

        return statement;
    }

    Uptr<GotoStatement> Parser::parseGotoStatement()
    {
        if (!extensionEnabled(JavacExtension::AllowGoto) && checkKeyword(GOTO))
        {
            throw parser_exception(ParserExceptionType::GotoIsReserved, peek());
        }

        Uptr<GotoStatement> statement = makeUniq<GotoStatement>();
        statement->range = expectKeyword(GOTO).range;

        const Token& label = expectIdentifier();
        statement->label = label.content;
        statement->range->end = label.range.end;

        statement->range->end = expectSymbol(SymbolType::Semicolon).range.end;

        return statement;
    }

    Uptr<AssertStatement> Parser::parseAssertStatement()
    {
        Uptr<AssertStatement> statement = makeUniq<AssertStatement>();

        statement->range = expectKeyword(ASSERT).range;

        statement->condition = parseValue();

        if (matchSymbol(SymbolType::Colon))
        {
            statement->message = parseValue();
        }

        statement->range->end = expectSymbol(SymbolType::Semicolon).range.end;

        return statement;
    }

    Uptr<BreakStatement> Parser::parseBreakStatement()
    {
        Uptr<BreakStatement> statement = makeUniq<BreakStatement>();
        statement->range = expectKeyword(BREAK).range;

        if (checkIdentifier())
        {
            const Token& label = expectIdentifier();
            statement->label = label.content;
            statement->range->end = label.range.end;
        }

        statement->range->end = expectSymbol(SymbolType::Semicolon).range.end;

        return statement;
    }

    Uptr<SynchronizedStatement> Parser::parseSynchronizedStatement()
    {
        Uptr<SynchronizedStatement> statement = makeUniq<SynchronizedStatement>();

        statement->range = expectKeyword(SYNCHRONIZED).range;

        expectSymbol(SymbolType::LParen);
        statement->monitorExpr = parseValue();
        expectSymbol(SymbolType::RParen);

        statement->body = parseBlock();
        statement->range->end = statement->body->range->end;

        return statement;
    }

    Uptr<TryStatement> Parser::parseTryStatement()
    {
        Uptr<TryStatement> statement = makeUniq<TryStatement>();
        statement->range = expectKeyword(TRY).range;

        if (matchSymbol(SymbolType::LParen))
        {
            do
            {
                statement->resources.push_back(parseTryResource());
            } while (matchSymbol(SymbolType::Semicolon) && !checkSymbol(SymbolType::RParen));
            expectSymbol(SymbolType::RParen);
        }

        statement->tryBlock = parseBlock();
        statement->range->end = statement->tryBlock->range->end;

        while (checkKeyword(CATCH))
        {
            statement->catches.push_back(parseCatchClause());
            statement->range->end = statement->catches.back().range->end;
        }

        if (matchKeyword(FINALLY))
        {
            statement->finallyBlock = parseBlock();
            statement->range->end = statement->finallyBlock->range->end;
        }
        else if (statement->resources.empty() && statement->catches.empty())
        {
            throw parser_exception(ParserExceptionType::ExpectCatchOrFinally, peek());
        }

        return statement;
    }

    TryResource Parser::parseTryResource()
    {
        //TODO: implement
    }
    Uptr<DeclStatement> Parser::parseVarDeclStatement()
    {
        //TODO: Parse a resource declaration and leave the separator to the caller.
        throw parser_exception(ParserExceptionType::UnsupportedStatement, peek());
    }
} // javac
