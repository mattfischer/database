#ifndef QUERY_HPP
#define QUERY_HPP

#include "Record.hpp"

#include <string>
#include <variant>

struct Query {
    struct CreateTable {
        std::string name;
        Record::Schema schema;
    };

    struct CreateIndex {
        std::string indexName;
        std::string tableName;
        std::vector<std::string> columns;
    };

    enum class Type {
        CreateTable,
        CreateIndex
    };

    Type type;
    std::variant<CreateTable, CreateIndex> query;
};

#endif