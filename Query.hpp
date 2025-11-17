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

    struct Update {
        std::string tableName;
        std::unique_ptr<Expression> predicate;
        std::vector<std::tuple<std::string, std::unique_ptr<Expression>>> values;
    };

    enum class Type {
        CreateTable,
        CreateIndex,
        Insert,
        Select,
        Delete,
        Update
    };

    Type type;
    std::variant<CreateTable, CreateIndex, Insert, Select, Delete, Update> query;
};

#endif