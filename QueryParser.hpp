#ifndef QUERYPARSER_HPP
#define QUERYPARSER_HPP

#include "Query.hpp"
#include "Value.hpp"

#include <memory>
#include <string>
#include <optional>

class QueryParser {
public:
    QueryParser(const std::string &queryString);

    std::unique_ptr<Query> parse();
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

    std::unique_ptr<Query> parseQuery();
    std::unique_ptr<Query> parseCreateTable();
    std::unique_ptr<Query> parseCreateIndex();
    std::unique_ptr<Query> parseInsert();
    std::unique_ptr<Query> parseSelect();
    std::unique_ptr<Query> parseDelete();
    std::unique_ptr<Query> parseUpdate();

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