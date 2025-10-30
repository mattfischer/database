#include <iostream>

#include "Database.hpp"
#include "PageSet.hpp"
#include "Record.hpp"
#include "Table.hpp"

#include "RowIterators/AggregateIterator.hpp"
#include "RowIterators/ExtendedProjectIterator.hpp"
#include "RowIterators/IndexIterator.hpp"
#include "RowIterators/ProjectIterator.hpp"
#include "RowIterators/SelectIterator.hpp"
#include "RowIterators/SortIterator.hpp"
#include "RowIterators/TableIterator.hpp"

#include <iostream>
#include <sstream>

void printIterator(RowIterator &iterator)
{
    for(int i = 0; iterator.valid(); i++) {
        std::cout << i << ": ";
        for(int j=0; j<iterator.schema().fields.size(); j++) {
            Value value = iterator.getField(j);
            value.print();
            std::cout << " ";
        }
        std::cout << std::endl;

        iterator.next();
    }
}

void modifyIterator(RowIterator &iterator)
{
    while(iterator.valid()) {
        std::vector<RowIterator::ModifyEntry> entries;
        entries.push_back({2, Value(1000)});
        iterator.modify(std::move(entries));

        iterator.next();
    }
}

int main(int argc, char *argv[])
{
    Database database;

    RecordSchema schema;
    schema.fields.push_back({Value::Type::String, "name"});
    schema.fields.push_back({Value::Type::Int, "value"});
    schema.fields.push_back({Value::Type::Int, "value2"});

    database.addTable(std::move(schema));
    Table &table = database.table(0);

    std::vector<unsigned int> keys;
    keys.push_back(0);
    database.addIndex(table, std::move(keys));

    keys.push_back(1);
    database.addIndex(table, std::move(keys));

    srand(12345);
    for(int i=0; i<64; i++) {
        int key = rand() % 5;
        int key2 = rand() % 1000;
        RecordWriter writer(table.schema());
        std::stringstream ss;
        ss << "Row " << i;
        writer.setField(0, Value(ss.str()));
        writer.setField(1, Value(key));
        writer.setField(2, Value(key2));
        table.addRow(writer);
    }

    table.print();

    RowIterators::TableIterator tableIterator(table);
    tableIterator.start();
    printIterator(tableIterator);
    std::cout << std::endl;
    std::cout << "-----------" << std::endl;
    std::cout << std::endl;

    std::unique_ptr<RowIterator> tableIterator2 = std::make_unique<RowIterators::TableIterator>(table);
    std::unique_ptr<Expression> expression = std::make_unique<CompareExpression>(CompareExpression::CompareType::LessThan,
        std::make_unique<FieldExpression>(2),
        std::make_unique<ConstantExpression>(Value(500))
    );
    std::unique_ptr<RowIterator> selectIterator = std::make_unique<RowIterators::SelectIterator>(std::move(tableIterator2), std::move(expression));
    selectIterator->start();
    modifyIterator(*selectIterator);

    RowIterators::TableIterator tableIterator3(table);
    tableIterator3.start();
    printIterator(tableIterator3);
    std::cout << std::endl;
    std::cout << "-----------" << std::endl;
    std::cout << std::endl;

    return 0;
}