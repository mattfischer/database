#ifndef QUERYPARSER_HPP
#define QUERYPARSER_HPP

#include "Value.hpp"
#include "Record.hpp"
#include "Expression.hpp"

#include "Database.hpp"

#include <memory>
#include <string>
#include <optional>
#include <variant>

class QueryParser {
public:
    QueryParser(const std::string &queryString);

    std::unique_ptr<Database::Operation> parse();
    const std::string &errorMessage();

private:
    void skipWhitespace();
    bool isEnd();
    void throwExpected(const std::string &expected);

    bool matchLiteral(const std::string &literal);
    void expectLiteral(const std::string &literal);

    std::optional<std::string> matchIdentifier();
    std::string expectIdentifier();
    Value::Type expectType();
    std::optional<Value> matchValue();
    Value expectValue();

    std::unique_ptr<Database::Operation> parseOperation();
    std::unique_ptr<Database::Operation> parseCreateTable();
    std::unique_ptr<Database::Operation> parseCreateIndex();
    std::unique_ptr<Database::Operation> parseInsert();
    std::unique_ptr<Database::Operation> parseSelect();
    std::unique_ptr<Database::Operation> parseDelete();
    std::unique_ptr<Database::Operation> parseUpdate();

    std::unique_ptr<Expression> expectExpression();
    std::unique_ptr<Expression> parseOrExpression();
    std::unique_ptr<Expression> parseAndExpression();
    std::unique_ptr<Expression> parseCompareExpression();
    std::unique_ptr<Expression> parseAddSubExpression();
    std::unique_ptr<Expression> parseMulDivExpression();
    std::unique_ptr<Expression> parseUnaryExpression();
    std::unique_ptr<Expression> parseBaseExpression();

    std::string mQueryString;
    std::string mErrorMessage;
    unsigned int mPos;
};

#endif