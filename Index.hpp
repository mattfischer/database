#ifndef INDEX_HPP
#define INDEX_HPP

#include "Record.hpp"
#include "BTree.hpp"
#include "Table.hpp"

#include <vector>
#include <memory>
#include <span>

class Index {
public:
    Index(Page &rootPage, Table &table, std::vector<unsigned int> keys);

    typedef BTree::Pointer Pointer;

    struct Limit {
        BTree::SearchComparison comparison;
        BTree::SearchPosition position;
        std::vector<Value> values;
    };

    Table &table();
    Record::Schema &keySchema();

    void add(Table::RowId rowId, Record::Writer &writer);
    void modify(Table::RowId rowId, Record::Writer &writer);
    void remove(Table::RowId rowId, std::span<Pointer*> trackPointers);
    
    Pointer first();
    Pointer last();

    bool moveNext(Pointer &pointer);
    bool movePrev(Pointer &pointer);

    Pointer lookup(Limit &limit);
    Table::RowId rowId(Pointer pointer);

    void print();

private:
    int partialKeyCompare(BTree::Key a, BTree::Key b, int numFields);

    Table &mTable;
    std::vector<unsigned int> mKeys;
    Record::Schema mKeySchema;
    std::unique_ptr<BTree> mTree;
};
#endif