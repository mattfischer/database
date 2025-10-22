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

    std::unique_ptr<RowIterator> tableIterator2 = std::make_unique<RowIterators::TableIterator>(table);
    std::unique_ptr<Expression> expression = std::make_unique<ArithmeticExpression>(ArithmeticExpression::ArithmeticType::Add,
        std::make_unique<FieldExpression>(1),
        std::make_unique<ConstantExpression>(Value(10))
    );
    std::vector<RowIterators::ExtendedProjectIterator::FieldDefinition> fields;
    fields.push_back({"value", Value::Type::Int, std::move(expression)});
    RowIterators::ExtendedProjectIterator extendedProjectIterator(std::move(tableIterator2), std::move(fields));
    extendedProjectIterator.start();
    printIterator(extendedProjectIterator);

    return 0;
}