#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "PageSet.hpp"
#include "Table.hpp"
#include "Index.hpp"

#include <map>
#include <memory>

class Database {
public:
    PageSet &pageSet();

    void executeQuery(const std::string &queryString);

    void addTable(const std::string &name, Record::Schema schema);
    Table &table(const std::string &name);

    void addIndex(const std::string &name, const std::string &tableName, std::vector<unsigned int> keys);
    Index &index(const std::string &name);

private:
    PageSet mPageSet;
    std::map<std::string, std::unique_ptr<Table>> mTables;
    std::map<std::string, std::unique_ptr<Index>> mIndices;
};

#endif