#include <iostream>

#include "PageSet.hpp"
#include "Row.hpp"
#include "Table.hpp"

#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
    PageSet pageSet;
    RowSchema schema;
    schema.fields.push_back({RowSchema::Field::String, "name"});

    Page &rootPage = pageSet.addPage();
    Table table(rootPage, std::move(schema));
    table.initialize();

    srand(12345);
    for(int i=0; i<64; i++) {
        uint32_t key = rand() % 1000;
        RowWriter writer(table.schema());
        std::stringstream ss;
        ss << key;
        RowWriter::Field field;
        field.stringValue = ss.str();
        writer.setField(0, field);

        table.addRow(writer);
        table.print();
        std::cout << "----------" << std::endl;
    }

    return 0;
}