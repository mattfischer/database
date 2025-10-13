#ifndef TABLE_HPP
#define TABLE_HPP

#include "BTree.hpp"
#include "PageSet.hpp"
#include "Row.hpp"

#include <memory>

class Table {
public:
    typedef uint32_t RowId;

    Table(Page &rootPage, RowSchema schema);

    void initialize();

    RowSchema &schema();

    RowId addRow(RowWriter &writer);
    void removeRow(RowId rowId);

    void print();

private:
    PageSet &mPageSet;
    RowSchema mSchema;
    BTree mTree;
    RowId mNextRowId;
};
#endif