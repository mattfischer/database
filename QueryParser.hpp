#ifndef QUERYPARSER_HPP
#define QUERYPARSER_HPP

#include "Value.hpp"
#include "Record.hpp"
#include "Expression.hpp"

#include <memory>
#include <string>
#include <optional>
#include <variant>

struct ParsedQuery {
    struct CreateTable {
        std::string tableName;
        Record::Schema schema;
    };

    struct CreateIndex {
        std::string indexName;
        std::string tableName;
        std::vector<std::string> columns;
    };

    struct Insert {
        std::string tableName;
        std::vector<Value> values;
    };

    struct Select {
        struct AllColumns {};
        struct ColumnList {
            std::vector<std::tuple<std::string, std::unique_ptr<Expression>>> columns;
        };
        struct Aggregate {
            enum class Operation {
                Min,
                Average,
                Sum,
                Max,
                Count
            };
            Operation operation;
            std::string field;
            std::string groupField;
        };
        std::variant<AllColumns, ColumnList, Aggregate> columns;

        std::string tableName;
        std::unique_ptr<Expression> predicate;
        std::string sortField;
    };

    struct Delete {
        std::string tableName;
        std::unique_ptr<Expression> predicate;
    };

    struct Update {
        std::string tableName;
        std::unique_ptr<Expression> predicate;
        std::vector<std::tuple<std::string, std::unique_ptr<Expression>>> values;
    };

    std::variant<CreateTable, CreateIndex, Insert, Select, Delete, Update> query;
};

class QueryParser {
public:
    QueryParser(const std::string &queryString);

    std::unique_ptr<ParsedQuery> parse();
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

    std::unique_ptr<ParsedQuery> parseQuery();
    std::unique_ptr<ParsedQuery> parseCreateTable();
    std::unique_ptr<ParsedQuery> parseCreateIndex();
    std::unique_ptr<ParsedQuery> parseInsert();
    std::unique_ptr<ParsedQuery> parseSelect();
    std::unique_ptr<ParsedQuery> parseDelete();
    std::unique_ptr<ParsedQuery> parseUpdate();

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