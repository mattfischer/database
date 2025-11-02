#ifndef TABLE_HPP
#define TABLE_HPP

#include "BTree.hpp"
#include "PageSet.hpp"
#include "Record.hpp"

#include <memory>
#include <span>

class Index;
class Table {
public:
    typedef uint32_t RowId;
    typedef BTree::Pointer Pointer;

    Table(Page &rootPage, RecordSchema schema);

    void initialize();

    std::vector<Index*> &indices();

    RecordSchema &schema();

    RowId addRow(RecordWriter &writer);
    void modifyRow(RowId rowId, RecordWriter &writer);
    void modifyRow(Pointer pointer, RecordWriter &writer);    
    void removeRow(RowId rowId, std::span<Pointer*> trackPointers);
    void removeRow(Pointer pointer, std::span<Pointer*> trackPointers);

    Pointer first();
    Pointer last();

    bool moveNext(Pointer &pointer);
    bool movePrev(Pointer &pointer);

    Pointer lookup(RowId rowId);

    RowId getRowId(Pointer pointer);
    void *data(Pointer pointer);

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