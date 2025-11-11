#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "PageSet.hpp"
#include "Table.hpp"
#include "Index.hpp"
#include "Query.hpp"

#include <map>
#include <memory>

class Database {
public:
    struct QueryResult {
        std::string message;
    };

    PageSet &pageSet();

    QueryResult executeQuery(const std::string &queryString);

    Table &table(const std::string &name);
    Index &index(const std::string &name);

private:
    QueryResult createTable(Query::CreateTable &createTable);
    QueryResult createIndex(Query::CreateIndex &createIndex);

    PageSet mPageSet;
    std::map<std::string, std::unique_ptr<Table>> mTables;
    std::map<std::string, std::unique_ptr<Index>> mIndices;
};

#endif