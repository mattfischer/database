#include <iostream>

#include "PageSet.hpp"
#include "Record.hpp"
#include "Table.hpp"

#include "RowIterators/SelectIterator.hpp"
#include "RowIterators/TableIterator.hpp"

#include "RowPredicates/ComparePredicate.hpp"

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

    RowIterators::TableIterator tableIterator(table);
    tableIterator.start();
    printIterator(tableIterator);
    std::cout << std::endl;
    std::cout << "-----------" << std::endl;
    std::cout << std::endl;

    std::unique_ptr<RowPredicate> predicate = std::make_unique<RowPredicates::ComparePredicate>(1, RowPredicates::ComparePredicate::Comparison::LessThan, Value(500));
    RowIterators::SelectIterator selectIterator(std::make_unique<RowIterators::TableIterator>(table), std::move(predicate));
    selectIterator.start();
    printIterator(selectIterator);

    return 0;
}