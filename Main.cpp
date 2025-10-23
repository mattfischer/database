#include <iostream>

#include "PageSet.hpp"
#include "Record.hpp"
#include "Table.hpp"

#include "RowIterators/ExtendedProjectIterator.hpp"
#include "RowIterators/IndexIterator.hpp"
#include "RowIterators/ProjectIterator.hpp"
#include "RowIterators/SelectIterator.hpp"
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

int main(int argc, char *argv[])
{
    PageSet pageSet;
    RecordSchema schema;
    schema.fields.push_back({Value::Type::String, "name"});
    schema.fields.push_back({Value::Type::Int, "value"});

    Page &rootPage = pageSet.addPage();
    Table table(rootPage, std::move(schema));
    table.initialize();

    std::vector<unsigned int> keys;
    keys.push_back(0);
    table.addIndex(std::move(keys));

    keys.push_back(1);
    table.addIndex(std::move(keys));

    srand(12345);
    for(int i=0; i<64; i++) {
        int key = rand() % 1000;
        RecordWriter writer(table.schema());
        std::stringstream ss;
        ss << "Row " << i;
        writer.setField(0, Value(ss.str()));
        writer.setField(1, Value(key));
        table.addRow(writer);
    }
    table.print();
    table.indices()[0]->print();
    std::cout << std::endl;
    table.indices()[1]->print();
    std::cout << std::endl;

    RowIterators::TableIterator tableIterator(table);
    tableIterator.start();
    printIterator(tableIterator);
    std::cout << std::endl;
    std::cout << "-----------" << std::endl;
    std::cout << std::endl;

    RecordWriter startWriter(table.indices()[1]->keySchema());
    startWriter.setField(0, Value(500));
    RowIterators::IndexIterator::Limit startLimit;
    startLimit.comparison = BTree::SearchComparison::GreaterThanEqual;
    startLimit.position = BTree::SearchPosition::First;
    startLimit.key.data.resize(startWriter.dataSize());
    startWriter.write(startLimit.key.data.data());

    RecordWriter endWriter(table.indices()[1]->keySchema());
    endWriter.setField(0, Value(800));
    RowIterators::IndexIterator::Limit endLimit;
    endLimit.comparison = BTree::SearchComparison::LessThanEqual;
    endLimit.position = BTree::SearchPosition::Last;
    endLimit.key.data.resize(endWriter.dataSize());
    endWriter.write(endLimit.key.data.data());

    RowIterators::IndexIterator indexIterator(*table.indices()[1], startLimit, endLimit);
    indexIterator.start();
    printIterator(indexIterator);
    return 0;
}