#include "Database.hpp"

#include "Optimizer.hpp"
#include "Parser.hpp"

#include "RowIterators/TableIterator.hpp"
#include "RowIterators/IndexIterator.hpp"
#include "RowIterators/SelectIterator.hpp"
#include "RowIterators/SortIterator.hpp"
#include "RowIterators/ProjectIterator.hpp"
#include "RowIterators/AggregateIterator.hpp"

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
    Parser parser(queryString);

    std::unique_ptr<Operation> operation = parser.parse();
    if(!operation) {
        return {parser.errorMessage()};
    }

    try {
        if(std::holds_alternative<Operation::CreateTable>(operation->operation))
            return createTable(std::get<Operation::CreateTable>(operation->operation));
        else if(std::holds_alternative<Operation::CreateIndex>(operation->operation))
            return createIndex(std::get<Operation::CreateIndex>(operation->operation));
        else if(std::holds_alternative<Operation::Insert>(operation->operation))
            return insert(std::get<Operation::Insert>(operation->operation));
        else if(std::holds_alternative<Operation::Select>(operation->operation))
            return select(std::get<Operation::Select>(operation->operation));
        else if(std::holds_alternative<Operation::Delete>(operation->operation))
            return delete_(std::get<Operation::Delete>(operation->operation));
        else if(std::holds_alternative<Operation::Update>(operation->operation))
            return update(std::get<Operation::Update>(operation->operation));
        else return {};
    } catch(QueryError e) {
        return {e.message};
    }
}

Database::QueryResult Database::createTable(Operation::CreateTable &createTable)
{
    Page &rootPage = mPageSet.addPage();
    std::unique_ptr table = std::make_unique<Table>(rootPage, std::move(createTable.schema));
    table->initialize();
    mTables[createTable.tableName] = std::move(table);

    return {"Created table " + createTable.tableName};
}

Database::QueryResult Database::createIndex(Operation::CreateIndex &createIndex)
{
    Table &table = findTable(createIndex.tableName);

    if(mIndices.contains(createIndex.indexName)) {
        std::stringstream ss;
        ss << "Error: Index " << createIndex.indexName << " already exists";
        return {ss.str()};
    }

    std::vector<unsigned int> keys;
    for(auto &column : createIndex.columns) {
        unsigned int field = fieldIndex(column, table.schema(), createIndex.tableName);
        keys.push_back(field);
    }

    Page &rootPage = mPageSet.addPage();
    std::unique_ptr index = std::make_unique<Index>(rootPage, table, std::move(keys));
    table.addIndex(*index);

    mIndices[createIndex.indexName] = std::move(index);

    return {"Created index " + createIndex.indexName};
}

Database::QueryResult Database::insert(Operation::Insert &insert)
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

Database::QueryResult Database::select(Operation::Select &select)
{
    auto iterator = buildIterator(select.query);

    return {"", std::move(iterator)};
}

Database::QueryResult Database::delete_(Operation::Delete &delete_)
{
    auto iterator = buildIterator(delete_.query);

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

Database::QueryResult Database::update(Operation::Update &update)
{
    auto iterator = buildIterator(update.query);

    Record::Schema &schema = iterator->schema();

    std::vector<RowIterator::ModifyEntry> entries;
    for(auto &[name, expression] : update.values) {
        unsigned int field = fieldIndex(name, schema, tableName(update.query));
        bindExpression(*expression, schema, tableName(update.query));
        RowIterator::ModifyEntry entry = {field, std::move(expression)};
        entries.push_back(std::move(entry));
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

std::unique_ptr<RowIterator> Database::buildIterator(Query &query)
{
    Optimizer optimizer(*this);
    optimizer.optimize(query);

    std::unique_ptr<RowIterator> iterator;

    if(std::holds_alternative<Query::Table>(query.source)) {
        auto &table = std::get<Query::Table>(query.source);
        iterator = std::make_unique<RowIterators::TableIterator>(findTable(table.name));
    } else if(std::holds_alternative<Query::Index>(query.source)) {
        auto &index = std::get<Query::Index>(query.source);
        std::optional<RowIterators::IndexIterator::Limit> startLimit;
        if(index.startLimit) {
            startLimit = {index.startLimit->comparison, index.startLimit->position, std::move(index.startLimit->values)};
        }
        std::optional<RowIterators::IndexIterator::Limit> endLimit;
        if(index.endLimit) {
            endLimit = {index.endLimit->comparison, index.endLimit->position, std::move(index.endLimit->values)};
        }
        iterator = std::make_unique<RowIterators::IndexIterator>(findIndex(index.name), std::move(startLimit), std::move(endLimit));
    }

    Record::Schema &schema = iterator->schema();

    if(query.predicate) {
        bindExpression(*query.predicate, schema, tableName(query));
        iterator = std::make_unique<RowIterators::SelectIterator>(std::move(iterator), std::move(query.predicate));
    }

    if(std::holds_alternative<Query::Aggregate>(query.columns)) {
        auto &aggregate = std::get<Query::Aggregate>(query.columns);
        RowIterators::AggregateIterator::Operation operation;
        switch(aggregate.operation) {
            case Query::Aggregate::Operation::Min: operation = RowIterators::AggregateIterator::Operation::Min; break;
            case Query::Aggregate::Operation::Average: operation = RowIterators::AggregateIterator::Operation::Average; break;
            case Query::Aggregate::Operation::Sum: operation = RowIterators::AggregateIterator::Operation::Sum; break;
            case Query::Aggregate::Operation::Max: operation = RowIterators::AggregateIterator::Operation::Max; break;
            case Query::Aggregate::Operation::Count: operation = RowIterators::AggregateIterator::Operation::Count; break;
        };
        unsigned int field = -1;
        if(aggregate.operation != Query::Aggregate::Operation::Count) {
            field = fieldIndex(aggregate.field, schema, tableName(query));
        }
        unsigned int groupField = -1;
        if(aggregate.groupField != "") {
            groupField = fieldIndex(aggregate.groupField, schema, tableName(query));
            iterator = std::make_unique<RowIterators::SortIterator>(std::move(iterator), groupField);
        }
        iterator = std::make_unique<RowIterators::AggregateIterator>(std::move(iterator), operation, field, groupField);
    } else {
        if(!query.sortField.empty()) {
            unsigned int field = fieldIndex(query.sortField, schema, tableName(query));
            iterator = std::make_unique<RowIterators::SortIterator>(std::move(iterator), field);
        }

        if(std::holds_alternative<Query::ColumnList>(query.columns)) {
            auto &columnList = std::get<Query::ColumnList>(query.columns);
            std::vector<RowIterators::ProjectIterator::FieldDefinition> fields;
            for(auto &[name, expression] : columnList.columns) {
                bindExpression(*expression, schema, tableName(query));
                RowIterators::ProjectIterator::FieldDefinition field;
                field.name = name;
                field.expression = std::move(expression);
                fields.push_back(std::move(field));
            }
            iterator = std::make_unique<RowIterators::ProjectIterator>(std::move(iterator), std::move(fields));
        }
    }

    return iterator;
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

    std::tuple<int, Value::Type> field(const std::string &name) {
        int fieldIndex = mSchema.fieldIndex(name);
        Value::Type type = mSchema.fields[fieldIndex].type;

        return {fieldIndex, type};
    }

private:
    Record::Schema mSchema;
};

void Database::bindExpression(Expression &expression, Record::Schema &schema, const std::string &tableName)
{
    SchemaBindContext context(schema);
    try {
        expression.bind(context);
    } catch(Expression::BindError e) {
        std::stringstream ss;
        ss << "Error: No column named " << e.name << "in table " << tableName;
        throw QueryError {ss.str()};
    }
}

unsigned int Database::fieldIndex(const std::string &name, Record::Schema &schema, const std::string &tableName)
{
    int fieldIndex = schema.fieldIndex(name);
    if(fieldIndex == -1) {
        std::stringstream ss;
        ss << "Error: No column named " << name << "in table " << tableName;
        throw QueryError {ss.str()};
    }

    return (unsigned int)fieldIndex;
}

const std::string &Database::tableName(Query &query)
{
    if(std::holds_alternative<Query::Table>(query.source)) {
        return std::get<Query::Table>(query.source).name;
    } else if(std::holds_alternative<Query::Index>(query.source)) {
        return std::get<Query::Index>(query.source).name;
    }
}