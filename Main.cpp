#include <iostream>

#include "PageSet.hpp"
#include "Record.hpp"
#include "Table.hpp"

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
        table.print();
        std::cout << "----------" << std::endl;
    }

    Result result = table.allRows();
    for(int i = 0; result.valid(); i++) {
        void *data = result.data();
        RecordReader reader(table.schema(), data);
        std::cout << i << ": ";
        reader.print();
        std::cout << std::endl;

        result.next();
    }

    return 0;
}