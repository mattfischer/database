#include <iostream>

#include "PageSet.hpp"
#include "Record.hpp"
#include "Table.hpp"

#include "RowIterators/TableIterator.hpp"

#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
    PageSet pageSet;
    RecordSchema schema;
    schema.fields.push_back({Value::Type::String, "name"});

    Page &rootPage = pageSet.addPage();
    Table table(rootPage, std::move(schema));
    table.initialize();

    srand(12345);
    for(int i=0; i<64; i++) {
        uint32_t key = rand() % 1000;
        RecordWriter writer(table.schema());
        std::stringstream ss;
        ss << key;
        Value value(Value::Type::String);
        value.setStringValue(ss.str());
        writer.setField(0, value);

        table.addRow(writer);
    }

    RowIterators::TableIterator iterator(table);
    for(int i = 0; iterator.valid(); i++) {
        std::cout << i << ": ";
        for(int j=0; j<table.schema().fields.size(); j++) {
            Value value = iterator.getField(j);
            value.print();
            std::cout << " ";
        }
        std::cout << std::endl;

        iterator.next();
    }

    return 0;
}