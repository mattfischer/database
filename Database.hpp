#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "PageSet.hpp"
#include "Table.hpp"

#include <vector>
#include <memory>

class Database {
public:
    PageSet &pageSet();

    void addTable(RecordSchema schema);

    Table &table(unsigned int index);

private:
    PageSet mPageSet;
    std::vector<std::unique_ptr<Table>> mTables;
};

#endif