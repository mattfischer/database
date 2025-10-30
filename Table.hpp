#ifndef TABLE_HPP
#define TABLE_HPP

#include "BTree.hpp"
#include "PageSet.hpp"
#include "Record.hpp"

#include "Index.hpp"

#include <memory>
#include <span>

class Table {
public:
    typedef uint32_t RowId;

    Table(Page &rootPage, RecordSchema schema);

    void initialize();

    BTree &tree();
    std::vector<Index*> &indices();

    RecordSchema &schema();

    RowId addRow(RecordWriter &writer);
    void modifyRow(RowId rowId, RecordWriter &writer);
    void modifyRow(BTree::Pointer pointer, RecordWriter &writer);    
    void removeRow(RowId rowId, std::span<BTree::Pointer*> trackPointers);
    void removeRow(BTree::Pointer pointer, std::span<BTree::Pointer*> trackPointers);

    RowId getRowId(BTree::Pointer pointer);
    BTree::Pointer lookup(RowId rowId);
    void *data(BTree::Pointer pointer);

    void addIndex(Index &index);

    void print();

private:
    PageSet &mPageSet;
    RecordSchema mSchema;
    BTree mTree;
    RowId mNextRowId;
    std::vector<Index*> mIndices;
};
#endif