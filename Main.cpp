#include <iostream>

#include "Database.hpp"
#include "PageSet.hpp"
#include "Record.hpp"
#include "Table.hpp"
#include "Index.hpp"

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

    database.addTable("Table", std::move(schema));

    std::vector<unsigned int> keys = {1, 2};
    database.addIndex("Table.1", "Table", std::move(keys));

    Table &table = database.table("Table");
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

    RowIterators::TableIterator tableIterator(database.table("Table"));
    tableIterator.start();
    printIterator(tableIterator);
    std::cout << std::endl;
    std::cout << "-----------" << std::endl;
    std::cout << std::endl;

    Index &index = database.index("Table.1");
    RecordWriter startWriter(index.keySchema());
    startWriter.setField(0, Value(3));
    BTree::KeyValue startValue;
    startValue.data.resize(startWriter.dataSize());
    startWriter.write(startValue.data.data());
    RowIterators::IndexIterator::Limit startLimit = { BTree::SearchComparison::Equal, BTree::SearchPosition::First, startValue, 1 };
    
    RecordWriter endWriter(index.keySchema());
    endWriter.setField(0, Value(4));
    BTree::KeyValue endValue;
    endValue.data.resize(endWriter.dataSize());
    endWriter.write(endValue.data.data());
    RowIterators::IndexIterator::Limit endLimit = { BTree::SearchComparison::Equal, BTree::SearchPosition::Last, endValue, 1 };
    
    RowIterators::IndexIterator indexIterator(index, startLimit, endLimit);
    indexIterator.start();
    printIterator(indexIterator);

    return 0;
}