#include "Database.hpp"

PageSet &Database::pageSet()
{
    return mPageSet;
}

void Database::addTable(RecordSchema schema)
{
    Page &rootPage = mPageSet.addPage();
    std::unique_ptr table = std::make_unique<Table>(rootPage, std::move(schema));
    table->initialize();
    mTables.push_back(std::move(table));
}

Table &Database::table(unsigned int index)
{
    return *mTables[index];
}

void Database::addIndex(Table &table, std::vector<unsigned int> keys)
{
    std::unique_ptr index = std::make_unique<Index>(table, std::move(keys));
    table.addIndex(*index);

    mIndices.push_back(std::move(index));
}
