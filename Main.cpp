#include <iostream>

#include "PageSet.hpp"
#include "BTree.hpp"
#include "Row.hpp"

#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
    PageSet pageSet;
    Page &page = pageSet.addPage();
    std::unique_ptr<TreePage::KeyDefinition> keyDefinition = std::make_unique<BTree::RowIdKeyDefinition>();

    BTree tree(pageSet, page.index(), std::move(keyDefinition));
    tree.initialize();

    RowSchema schema;
    schema.fields.push_back({RowSchema::Field::String, "name"});

    auto printCell = [&](void *data) {
        RowReader reader(schema, data);
        for(unsigned int i=0; i<schema.fields.size(); i++) {
            switch(schema.fields[i].type) {
                case RowSchema::Field::Int:
                    std::cout << reader.readInt(i) << " ";
                    break;
                case RowSchema::Field::Float:
                    std::cout << reader.readFloat(i) << " ";
                    break;
                case RowSchema::Field::String:
                    std::cout << "\"" << reader.readString(i) << "\" ";
                    break;
            }
        }
    };

    srand(12345);
    for(int i=0; i<64; i++) {
        uint32_t key = rand() % 1000;
        RowWriter writer(schema);
        std::stringstream ss;
        ss << "Row " << i;
        RowWriter::Field field;
        field.stringValue = ss.str();
        writer.setField(0, field);

        void *data = tree.add(&key, sizeof(key), writer.dataSize());
        writer.write(data);

        tree.print(printCell);
        std::cout << "----------" << std::endl;
    }

    srand(12345);
    for(int i=0; i<64; i++) {
        uint32_t key = rand() % 1000;
        tree.remove(&key);
        tree.print(printCell);
        std::cout << "----------" << std::endl;
    }

    return 0;
}