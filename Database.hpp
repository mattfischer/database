#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "PageSet.hpp"
#include "Table.hpp"
#include "Index.hpp"
#include "RowIterator.hpp"
#include "QueryParser.hpp"

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
    QueryResult createTable(ParsedQuery::CreateTable &createTable);
    QueryResult createIndex(ParsedQuery::CreateIndex &createIndex);
    QueryResult insert(ParsedQuery::Insert &insert);
    QueryResult select(ParsedQuery::Select &select);
    QueryResult delete_(ParsedQuery::Delete &delete_);
    QueryResult update(ParsedQuery::Update &update);

    PageSet mPageSet;
    std::map<std::string, std::unique_ptr<Table>> mTables;
    std::map<std::string, std::unique_ptr<Index>> mIndices;
};

#endif