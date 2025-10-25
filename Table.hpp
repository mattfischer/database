#ifndef TABLE_HPP
#define TABLE_HPP

#include "BTree.hpp"
#include "PageSet.hpp"
#include "Record.hpp"

#include "Index.hpp"

#include <memory>

class Table {
public:
    typedef uint32_t RowId;

    Table(Page &rootPage, RecordSchema schema);

    void initialize();

    BTree &tree();
    std::vector<std::unique_ptr<Index>> &indices();

    RecordSchema &schema();

    RowId addRow(RecordWriter &writer);
    void modifyRow(RowId rowId, RecordWriter &writer);    
    void removeRow(RowId rowId, BTree::Pointer &trackPointer);
    void removeRow(BTree::Pointer &pointer);

    RowId getRowId(BTree::Pointer pointer);

    void addIndex(std::vector<unsigned int> keys);

    void print();

private:
    PageSet &mPageSet;
    RecordSchema mSchema;
    BTree mTree;
    RowId mNextRowId;
    std::vector<std::unique_ptr<Index>> mIndices;
};
#endif