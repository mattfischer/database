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

    class RecordKey {
    public:
        RecordKey(Record::Writer &writer) {
            mData.resize(writer.dataSize());
            writer.write(mData.data());
        }

        operator BTree::Key() {
            return BTree::Key(mData.data(), mData.size());
        }
 
    private:
        std::vector<uint8_t> mData;
    };

    typedef BTree::Pointer Pointer;

    Table &table();
    Record::Schema &keySchema();

    void add(Table::RowId rowId, Record::Writer &writer);
    void modify(Table::RowId rowId, Record::Writer &writer);
    void remove(Table::RowId rowId, std::span<Pointer*> trackPointers);
    
    Pointer first();
    Pointer last();

    bool moveNext(Pointer &pointer);
    bool movePrev(Pointer &pointer);

    Pointer lookup(BTree::Key key, BTree::KeyComparator &comparator, BTree::SearchComparison comparison, BTree::SearchPosition position);
    Table::RowId rowId(Pointer pointer);

    void print();

private:
    Table &mTable;
    std::vector<unsigned int> mKeys;
    Record::Schema mKeySchema;
    std::unique_ptr<BTree> mTree;
};
#endif