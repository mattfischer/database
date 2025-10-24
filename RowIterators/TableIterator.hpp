#ifndef ROWITERATORS_TABLEITERATOR_HPP
#define ROWITERATORS_TABLEITERATOR_HPP

#include "RowIterator.hpp"
#include "Table.hpp"

namespace RowIterators {
    class TableIterator : public RowIterator {
    public:
        TableIterator(Table &table);

        RecordSchema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;
        bool remove() override;
        bool modify(std::vector<ModifyEntry> entries) override;

        Value getField(unsigned int index) override;

    private:
        Table &mTable;
        BTree::Pointer mPointer; 
    };
}
#endif