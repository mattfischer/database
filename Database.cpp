#include "Database.hpp"

#include "QueryParser.hpp"

PageSet &Database::pageSet()
{
    return mPageSet;
}

void Database::executeQuery(const std::string &queryString)
{
    QueryParser parser(queryString);

    std::unique_ptr<Query> query = parser.parse();
    if(!query) {
        return;
    }

    switch(query->type) {
        case Query::Type::CreateTable:
        {
            auto &createTable = std::get<Query::CreateTable>(query->query);
            addTable(createTable.name, createTable.schema);
            break;
        }
    }
}
void Database::addTable(const std::string &name, Record::Schema schema)
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
    Page &rootPage = mPageSet.addPage();
    Table &table = *mTables[tableName];
    std::unique_ptr index = std::make_unique<Index>(rootPage, table, std::move(keys));
    table.addIndex(*index);

    mIndices[name] = std::move(index);
}

Index &Database::index(const std::string &name)
{
    return *mIndices[name];
}
