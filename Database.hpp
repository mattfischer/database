#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "PageSet.hpp"
#include "Table.hpp"
#include "Index.hpp"
#include "Query.hpp"
#include "RowIterator.hpp"

#include <map>
#include <memory>

class Database {
public:
    struct QueryResult {
        std::string message;
        std::unique_ptr<RowIterator> iterator;
    };

    PageSet &pageSet();

    QueryResult executeQuery(const std::string &queryString);

    Table &table(const std::string &name);
    Index &index(const std::string &name);

private:
    QueryResult createTable(Query::CreateTable &createTable);
    QueryResult createIndex(Query::CreateIndex &createIndex);
    QueryResult insert(Query::Insert &insert);
    QueryResult select(Query::Select &select);
    QueryResult delete_(Query::Delete &delete_);
    QueryResult update(Query::Update &update);

    PageSet mPageSet;
    std::map<std::string, std::unique_ptr<Table>> mTables;
    std::map<std::string, std::unique_ptr<Index>> mIndices;
};

#endif