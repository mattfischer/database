#ifndef ROWITERATORS_FOREIGNKEYJOINITERATOR_HPP
#define ROWITERATORS_FOREIGNKEYJOINITERATOR_HPP

#include "RowIterator.hpp"
#include "Table.hpp"

namespace RowIterators {
    class ForeignKeyJoinIterator : public RowIterator {
    public:
        ForeignKeyJoinIterator(std::unique_ptr<RowIterator> inputIterator, unsigned int foreignKeyIndex, Table &foreignTable);

        RecordSchema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;
        bool remove() override;

        Value getField(unsigned int index) override;

    private:
        void updateTablePointer();

        RecordSchema mSchema;
        std::unique_ptr<RowIterator> mInputIterator;
        unsigned int mForeignKeyIndex;
        Table &mForeignTable;
        BTree::Pointer mTablePointer;
    };
}
#endif