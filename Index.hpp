#ifndef INDEX_HPP
#define INDEX_HPP

#include "Record.hpp"
#include "BTree.hpp"

#include <vector>
#include <memory>

class Table;
class Index {
public:
    Index(Table &table, std::vector<unsigned int> keys);

    typedef uint32_t RowId;

    Table &table();
    BTree &tree();
    RecordSchema &keySchema();

    void add(RecordWriter &writer, RowId rowId);
    void remove(RowId rowId);
    void print();

private:
    Table &mTable;
    std::vector<unsigned int> mKeys;
    RecordSchema mKeySchema;
    std::unique_ptr<BTree> mTree;
};
#endif