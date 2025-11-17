#ifndef QUERY_HPP
#define QUERY_HPP

#include "Record.hpp"
#include "Expression.hpp"

#include <string>
#include <variant>

struct Query {
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
        std::string tableName;
        std::unique_ptr<Expression> predicate;
    };

    struct Delete {
        std::string tableName;
        std::unique_ptr<Expression> predicate;
    };

    enum class Type {
        CreateTable,
        CreateIndex,
        Insert,
        Select,
        Delete
    };

    Type type;
    std::variant<CreateTable, CreateIndex, Insert, Select, Delete> query;
};

#endif