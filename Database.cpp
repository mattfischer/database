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