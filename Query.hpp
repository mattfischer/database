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

    enum class Type {
        CreateTable
    };

    Type type;
    std::variant<CreateTable> query;
};

#endif