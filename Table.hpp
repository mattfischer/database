#ifndef TABLE_HPP
#define TABLE_HPP

#include "BTree.hpp"
#include "PageSet.hpp"
#include "Record.hpp"

#include <memory>

class Table {
public:
    typedef uint32_t RowId;

    Table(Page &rootPage, RecordSchema schema);

    void initialize();

    RecordSchema &schema();

    RowId addRow(RecordWriter &writer);
    void removeRow(RowId rowId);

    void print();

private:
    PageSet &mPageSet;
    RecordSchema mSchema;
    BTree mTree;
    RowId mNextRowId;
};
#endif