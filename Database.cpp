#include "Database.hpp"

#include "QueryParser.hpp"

#include "RowIterators/TableIterator.hpp"
#include "RowIterators/SelectIterator.hpp"
#include "RowIterators/SortIterator.hpp"

#include <sstream>
#include <ranges>

PageSet &Database::pageSet()
{
    return mPageSet;
}

struct QueryError {
    std::string message;
};

Database::QueryResult Database::executeQuery(const std::string &queryString)
{
    QueryParser parser(queryString);

    std::unique_ptr<ParsedQuery> query = parser.parse();
    if(!query) {
        return {parser.errorMessage()};
    }

    try {
        switch(query->type) {
            case ParsedQuery::Type::CreateTable:
                return createTable(std::get<ParsedQuery::CreateTable>(query->query));
            case ParsedQuery::Type::CreateIndex:
                return createIndex(std::get<ParsedQuery::CreateIndex>(query->query));
            case ParsedQuery::Type::Insert:
                return insert(std::get<ParsedQuery::Insert>(query->query));
            case ParsedQuery::Type::Select:
                return select(std::get<ParsedQuery::Select>(query->query));
            case ParsedQuery::Type::Delete:
                return delete_(std::get<ParsedQuery::Delete>(query->query));
            case ParsedQuery::Type::Update:
                return update(std::get<ParsedQuery::Update>(query->query));
            default:
                return {};
        }
    } catch(QueryError e) {
        return {e.message};
    }
}

Database::QueryResult Database::createTable(ParsedQuery::CreateTable &createTable)
{
    Page &rootPage = mPageSet.addPage();
    std::unique_ptr table = std::make_unique<Table>(rootPage, std::move(createTable.schema));
    table->initialize();
    mTables[createTable.tableName] = std::move(table);

    return {"Created table " + createTable.tableName};
}

Database::QueryResult Database::createIndex(ParsedQuery::CreateIndex &createIndex)
{
    Table &table = findTable(createIndex.tableName);

    if(mIndices.contains(createIndex.indexName)) {
        std::stringstream ss;
        ss << "Error: Index " << createIndex.indexName << " already exists";
        return {ss.str()};
    }

    std::vector<unsigned int> keys;
    for(auto &column : createIndex.columns) {
        unsigned int fieldIndex = tableFieldIndex(column, table, createIndex.tableName);
        keys.push_back(fieldIndex);
    }

    Page &rootPage = mPageSet.addPage();
    std::unique_ptr index = std::make_unique<Index>(rootPage, table, std::move(keys));
    table.addIndex(*index);

    mIndices[createIndex.indexName] = std::move(index);

    return {"Created index " + createIndex.indexName};
}

Database::QueryResult Database::insert(ParsedQuery::Insert &insert)
{
    Table &table = findTable(insert.tableName);

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

Database::QueryResult Database::select(ParsedQuery::Select &select)
{
    Table &table = findTable(select.tableName);

    std::unique_ptr<RowIterator> iterator = std::make_unique<RowIterators::TableIterator>(table);

    if(select.predicate) {
        bindExpression(*select.predicate, table, select.tableName);
        iterator = std::make_unique<RowIterators::SelectIterator>(std::move(iterator), std::move(select.predicate));
    }

    if(!select.sortField.empty()) {
        unsigned int field = tableFieldIndex(select.sortField, table, select.tableName);
        iterator = std::make_unique<RowIterators::SortIterator>(std::move(iterator), field);
    }

    return {"", std::move(iterator)};
}

Database::QueryResult Database::delete_(ParsedQuery::Delete &delete_)
{
    Table &table = findTable(delete_.tableName);

    std::unique_ptr<RowIterator> iterator = std::make_unique<RowIterators::TableIterator>(table);

    if(delete_.predicate) {
        bindExpression(*delete_.predicate, table, delete_.tableName);
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

Database::QueryResult Database::update(ParsedQuery::Update &update)
{
    Table &table = findTable(update.tableName);

    std::vector<RowIterator::ModifyEntry> entries;
    for(auto &[name, expression] : update.values) {
        unsigned int fieldIndex = tableFieldIndex(name, table, update.tableName);
        bindExpression(*expression, table, update.tableName);
        RowIterator::ModifyEntry entry = {fieldIndex, std::move(expression)};
        entries.push_back(std::move(entry));
    }

    std::unique_ptr<RowIterator> iterator = std::make_unique<RowIterators::TableIterator>(table);

    if(update.predicate) {
        bindExpression(*update.predicate, table, update.tableName);
        iterator = std::make_unique<RowIterators::SelectIterator>(std::move(iterator), std::move(update.predicate));
    }

    int rowsUpdated = 0;
    iterator->start();
    while(iterator->valid()) {
        iterator->modify(entries);
        iterator->next();
        rowsUpdated++;
    }
    std::stringstream ss;
    ss << rowsUpdated << " rows updated";
    return {ss.str()};
}

Table &Database::findTable(const std::string &name)
{
    auto it = mTables.find(name);
    if(it == mTables.end()) {
        std::stringstream ss;
        ss << "Error: Table " << name << " does not exist";
        throw QueryError {ss.str()};
    }

    return *it->second;
}

Index &Database::findIndex(const std::string &name)
{
    auto it = mIndices.find(name);
    if(it == mIndices.end()) {
        std::stringstream ss;
        ss << "Error: Index " << name << " does not exist";
        throw QueryError {ss.str()};
    }

    return *it->second;
}

class SchemaBindContext : public Expression::BindContext {
public:
    SchemaBindContext(Record::Schema &schema)
    : mSchema(schema)
    {
    }

    int field(const std::string &name) {
        return mSchema.fieldIndex(name);
    }

private:
    Record::Schema mSchema;
};

void Database::bindExpression(Expression &expression, Table &table, const std::string &tableName)
{
    SchemaBindContext context(table.schema());
    try {
        expression.bind(context);
    } catch(Expression::BindError e) {
        std::stringstream ss;
        ss << "Error: No column named " << e.name << "in table " << tableName;
        throw QueryError {ss.str()};
    }
}

unsigned int Database::tableFieldIndex(const std::string &name, Table &table, const std::string &tableName)
{
    int fieldIndex = table.schema().fieldIndex(name);
    if(fieldIndex == -1) {
        std::stringstream ss;
        ss << "Error: No column named " << name << "in table " << tableName;
        throw QueryError {ss.str()};
    }

    return (unsigned int)fieldIndex;
}
