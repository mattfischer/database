#ifndef QUERYPARSER_HPP
#define QUERYPARSER_HPP

#include "Query.hpp"
#include "Value.hpp"

#include <memory>
#include <string>

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

    std::string expectIdentifier();
    Value::Type expectType();

    std::unique_ptr<Query> parseQuery();
    std::unique_ptr<Query> parseCreateTable();
    std::unique_ptr<Query> parseCreateIndex();

    std::string mQueryString;
    std::string mErrorMessage;
    unsigned int mPos;
};

#endif