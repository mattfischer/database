#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "PageSet.hpp"
#include "Table.hpp"
#include "Index.hpp"
#include "RowIterator.hpp"

#include <map>
#include <memory>
#include <optional>

class Database {
public:
    struct Query {
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

        struct Table {
            std::string name;
        };
        struct Index {
            struct Limit {
                BTree::SearchComparison comparison;
                BTree::SearchPosition position;
                std::vector<Value> values;
            };
            std::string name;
            std::optional<Limit> startLimit;
            std::optional<Limit> endLimit;
        };
        std::variant<Table, Index> source;

        std::unique_ptr<Expression> predicate;
        std::string sortField;
    };

    struct Operation {
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

        struct Delete {
            Query query;
        };

        struct Select {
            Query query;
        };

        struct Update {
            Query query;
            std::vector<std::tuple<std::string, std::unique_ptr<Expression>>> values;
        };

        std::variant<CreateTable, CreateIndex, Insert, Select, Delete, Update> operation;
    };

    struct QueryResult {
        std::string message;
        std::unique_ptr<RowIterator> iterator;
    };

    PageSet &pageSet();

    QueryResult executeQuery(const std::string &queryString);

private:
    QueryResult createTable(Operation::CreateTable &createTable);
    QueryResult createIndex(Operation::CreateIndex &createIndex);
    QueryResult insert(Operation::Insert &insert);
    QueryResult select(Operation::Select &select);
    QueryResult delete_(Operation::Delete &delete_);
    QueryResult update(Operation::Update &update);

    std::unique_ptr<RowIterator> buildIterator(Query &query);

    Table &findTable(const std::string &name);
    Index &findIndex(const std::string &name);
    void bindExpression(Expression &expression, Record::Schema &schema, const std::string &tableName);
    unsigned int fieldIndex(const std::string &name, Record::Schema &schema, const std::string &tableName);
    const std::string &tableName(Query &query);

    PageSet mPageSet;
    std::map<std::string, std::unique_ptr<Table>> mTables;
    std::map<std::string, std::unique_ptr<Index>> mIndices;
};

#endif