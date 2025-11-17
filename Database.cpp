#include "Database.hpp"

#include "QueryParser.hpp"

#include "RowIterators/TableIterator.hpp"
#include "RowIterators/SelectIterator.hpp"

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
        case Query::Type::Insert:
            return insert(std::get<Query::Insert>(query->query));
        case Query::Type::Select:
            return select(std::get<Query::Select>(query->query));
        case Query::Type::Delete:
            return delete_(std::get<Query::Delete>(query->query));
        default:
            return {};
    }
}

Database::QueryResult Database::createTable(Query::CreateTable &createTable)
{
    Page &rootPage = mPageSet.addPage();
    std::unique_ptr table = std::make_unique<Table>(rootPage, std::move(createTable.schema));
    table->initialize();
    mTables[createTable.tableName] = std::move(table);

    return {"Created table " + createTable.tableName};
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

Database::QueryResult Database::insert(Query::Insert &insert)
{
    auto it = mTables.find(insert.tableName);
    if(it == mTables.end()) {
        std::stringstream ss;
        ss << "Error: Table " << insert.tableName << " does not exist";
        return {ss.str()};
    }
    Table &table = *it->second;

    if(insert.values.size() != table.schema().fields.size()) {
        std::stringstream ss;
        ss << "Error: Incorrect number of values for table " << insert.tableName;
        return {ss.str()};
    }

    Record::Writer writer(table.schema());
    for(int i=0; i<insert.values.size(); i++) {
        if(insert.values[i].type() != table.schema().fields[i].type) {
            std::stringstream ss;
            ss << "Error: Incorrect type for column " << table.schema().fields[i].name << " in table " << insert.tableName;
            return {ss.str()};
        }

        writer.setField(i, insert.values[i]);
    }

    table.addRow(writer);

    return {"Added row to table " + insert.tableName};
}

class SchemaBindContext : public Expression::BindContext {
public:
    SchemaBindContext(Record::Schema &schema)
    : mSchema(schema)
    {
    }

    int field(const std::string &name) {
        auto it = std::ranges::find_if(mSchema.fields, [&](const auto &a) { return a.name == name; });
        if(it == mSchema.fields.end()) {
            return -1;
        } else {
            return it - mSchema.fields.begin();
        }
    }

private:
    Record::Schema mSchema;
};

Database::QueryResult Database::select(Query::Select &select)
{
    auto it = mTables.find(select.tableName);
    if(it == mTables.end()) {
        std::stringstream ss;
        ss << "Error: Table " << select.tableName << " does not exist";
        return {ss.str()};
    }
    Table &table = *it->second;

    if(select.predicate) {
        SchemaBindContext context(table.schema());
        try {
            select.predicate->bind(context);
        } catch(Expression::BindError e) {
            std::stringstream ss;
            ss << "Error: No column named " << e.name << "in table " << select.tableName;
            return {ss.str()};
        }
    }

    std::unique_ptr<RowIterator> iterator = std::make_unique<RowIterators::TableIterator>(table);
    if(select.predicate) {
        iterator = std::make_unique<RowIterators::SelectIterator>(std::move(iterator), std::move(select.predicate));
    }

    return {"", std::move(iterator)};
}

Database::QueryResult Database::delete_(Query::Delete &delete_)
{
    auto it = mTables.find(delete_.tableName);
    if(it == mTables.end()) {
        std::stringstream ss;
        ss << "Error: Table " << delete_.tableName << " does not exist";
        return {ss.str()};
    }
    Table &table = *it->second;

    if(delete_.predicate) {
        SchemaBindContext context(table.schema());
        try {
            delete_.predicate->bind(context);
        } catch(Expression::BindError e) {
            std::stringstream ss;
            ss << "Error: No column named " << e.name << "in table " << delete_.tableName;
            return {ss.str()};
        }
    }

    std::unique_ptr<RowIterator> iterator = std::make_unique<RowIterators::TableIterator>(table);
    if(delete_.predicate) {
        iterator = std::make_unique<RowIterators::SelectIterator>(std::move(iterator), std::move(delete_.predicate));
    }

    int rowsRemoved = 0;
    iterator->start();
    while(iterator->valid()) {
        iterator->remove();
        rowsRemoved++;
    }
    std::stringstream ss;
    ss << rowsRemoved << " rows removed";
    return {ss.str()};
}

Table &Database::table(const std::string &name)
{
    return *mTables[name];
}

Index &Database::index(const std::string &name)
{
    return *mIndices[name];
}
