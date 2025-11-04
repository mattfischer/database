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

    Table(Page &rootPage, Record::Schema schema);

    void initialize();

    std::vector<Index*> &indices();

    Record::Schema &schema();

    RowId addRow(Record::Writer &writer);
    void modifyRow(RowId rowId, Record::Writer &writer);
    void modifyRow(Pointer pointer, Record::Writer &writer);    
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
    Record::Schema mSchema;
    BTree mTree;
    RowId mNextRowId;
    std::vector<Index*> mIndices;
};
#endif