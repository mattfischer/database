#include <iostream>

#include "PageSet.hpp"
#include "Record.hpp"
#include "Table.hpp"

#include "RowIterators/ProjectIterator.hpp"
#include "RowIterators/SelectIterator.hpp"
#include "RowIterators/TableIterator.hpp"

#include "RowPredicates/ComparePredicate.hpp"
#include "RowPredicates/LogicalPredicate.hpp"

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
    table.indices()[0]->print();

    RowIterators::TableIterator tableIterator(table);
    tableIterator.start();
    printIterator(tableIterator);
    std::cout << std::endl;
    std::cout << "-----------" << std::endl;
    std::cout << std::endl;

    std::unique_ptr<RowPredicate> predicate1 = std::make_unique<RowPredicates::ComparePredicate>(1, RowPredicates::ComparePredicate::Comparison::LessThan, Value(100));
    std::unique_ptr<RowPredicate> predicate2 = std::make_unique<RowPredicates::ComparePredicate>(1, RowPredicates::ComparePredicate::Comparison::GreaterThan, Value(800));
    std::unique_ptr<RowPredicate> predicate = std::make_unique<RowPredicates::LogicalPredicate>(std::move(predicate1), RowPredicates::LogicalPredicate::Operation::Or, std::move(predicate2));

    std::unique_ptr<RowIterator> selectIterator = std::make_unique<RowIterators::SelectIterator>(std::make_unique<RowIterators::TableIterator>(table), std::move(predicate));
    std::vector<RowIterators::ProjectIterator::FieldDefinition> fields;
    fields.push_back({1, "value"});
    fields.push_back({0, "name"});
    RowIterators::ProjectIterator projectIterator(std::move(selectIterator), std::move(fields));
    projectIterator.start();
    printIterator(projectIterator);

    return 0;
}