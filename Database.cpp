#include "Database.hpp"

PageSet &Database::pageSet()
{
    return mPageSet;
}

void Database::addTable(const std::string &name, RecordSchema schema)
{
    Page &rootPage = mPageSet.addPage();
    std::unique_ptr table = std::make_unique<Table>(rootPage, std::move(schema));
    table->initialize();
    mTables[name] = std::move(table);
}

Table &Database::table(const std::string &name)
{
    return *mTables[name];
}

void Database::addIndex(const std::string &name, const std::string &tableName, std::vector<unsigned int> keys)
{
    Table &table = *mTables[tableName];
    std::unique_ptr index = std::make_unique<Index>(table, std::move(keys));
    table.addIndex(*index);

    mIndices[name] = std::move(index);
}

Index &Database::index(const std::string &name)
{
    return *mIndices[name];
}
