#include "Database.hpp"

#include "QueryParser.hpp"

#include <sstream>
#include <ranges>

PageSet &Database::pageSet()
{
    return mPageSet;
}

Database::QueryResult Database::executeQuery(const std::string &queryString)
{
    QueryParser parser(queryString);

    std::unique_ptr<Query> query = parser.parse();
    if(!query) {
        return {parser.errorMessage()};
    }

    switch(query->type) {
        case Query::Type::CreateTable:
            return createTable(std::get<Query::CreateTable>(query->query));
        case Query::Type::CreateIndex:
            return createIndex(std::get<Query::CreateIndex>(query->query));
        default:
            return {};
    }
}

Database::QueryResult Database::createTable(Query::CreateTable &createTable)
{
    Page &rootPage = mPageSet.addPage();
    std::unique_ptr table = std::make_unique<Table>(rootPage, std::move(createTable.schema));
    table->initialize();
    mTables[createTable.name] = std::move(table);

    return {"Created table " + createTable.name};
}

Database::QueryResult Database::createIndex(Query::CreateIndex &createIndex)
{
    auto it = mTables.find(createIndex.tableName);
    if(it == mTables.end()) {
        std::stringstream ss;
        ss << "Error: Table " << createIndex.tableName << " does not exist";
        return {ss.str()};
    }
    Table &table = *it->second;

    if(mIndices.contains(createIndex.indexName)) {
        std::stringstream ss;
        ss << "Error: Index " << createIndex.indexName << " already exists";
        return {ss.str()};
    }

    std::vector<unsigned int> keys;
    for(auto &column : createIndex.columns) {
        auto it = std::ranges::find_if(table.schema().fields, [&](const auto &a) { return a.name == column; });
        if(it == table.schema().fields.end()) {
            std::stringstream ss;
            ss << "Error: No column named  " << column << " in table " << createIndex.tableName;
            return {ss.str()};
        } else {
            keys.push_back(it - table.schema().fields.begin());
        }
    }

    Page &rootPage = mPageSet.addPage();
    std::unique_ptr index = std::make_unique<Index>(rootPage, table, std::move(keys));
    table.addIndex(*index);

    mIndices[createIndex.indexName] = std::move(index);

    return {"Created index " + createIndex.indexName};
}

Table &Database::table(const std::string &name)
{
    return *mTables[name];
}

Index &Database::index(const std::string &name)
{
    return *mIndices[name];
}
