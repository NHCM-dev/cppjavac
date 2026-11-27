#include "Parser.h"

#include <utility>

#include "ParserException.h"

namespace javac
{
    BlockStatement Parser::parseBlock()
    {
        BlockStatement blockStatement;
        blockStatement.range = expectSymbol(SymbolType::LBrace).range;

        while (!isAtEnd() && !checkSymbol(SymbolType::RBrace))
        {
            blockStatement.statements.push_back(parseStatement(true));
        }

        blockStatement.range->end = expectSymbol(SymbolType::RBrace).range.end;
        return blockStatement;
    }

    Uptr<Statement> Parser::parseStatement(bool allowBlock)
    {
        if (allowBlock && checkSymbol(SymbolType::LBrace))
        {
            return makeUniq<BlockStatement>(parseBlock());
        }

        if (checkSymbol(SymbolType::Semicolon))
        {
            auto statement = makeUniq<EmptyStatement>();
            statement->range = advance().range;
            return statement;
        }
        if (checkKeyword(THROW))
        {
            return parseThrowStatement();
        }
        if (checkKeyword(RETURN))
        {
            return parseReturnStatement();
        }
        if (checkKeyword(IF))
        {
            return parseIfStatement();
        }
        if (checkKeyword(SWITCH))
        {
            return parseSwitchStatement();
        }
        if (checkKeyword(CONTINUE))
        {
            return parseContinueStatement();
        }
        if (checkKeyword(GOTO))
        {
            return parseGotoStatement();
        }
        if (checkKeyword(ASSERT))
        {
            return parseAssertStatement();
        }
        if (checkKeyword(BREAK))
        {
            return parseBreakStatement();
        }
        if (checkKeyword(SYNCHRONIZED))
        {
            return parseSynchronizedStatement();
        }
        if (checkKeyword(TRY))
        {
            return parseTryStatement();
        }

        if (checkIdentifier() && peek(1).symbol == SymbolType::Colon)
        {
            return parseLabeledStatement();
        }

        std::vector<Annotation> pendingAnnotations = {};
        if (canBeAnnotation())
        {
            pendingAnnotations = parseAnnotations();
        }

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

        statement->thenBranch = parseStatement();
        if (matchKeyword(ELSE))
        {
            statement->elseBranch = parseStatement();
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
                    auto child = parseStatement();
                    branch.range->end = child->range->end;
                    branch.statements.push_back(std::move(child));
                }
            }
            else if (checkSymbol(SymbolType::LBrace))
            {
                branch.ruleStatement = makeUniq<BlockStatement>(parseBlock());
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
        statement->statement = parseStatement(true);
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

        statement->body = makeUniq<BlockStatement>(parseBlock());
        statement->range->end = statement->body->range->end;

        return statement;
    }

    Uptr<TryStatement> Parser::parseTryStatement()
    {
        Uptr<TryStatement> statement = makeUniq<TryStatement>();
        //TODO: implement
    }
} // javac
