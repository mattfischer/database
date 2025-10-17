#ifndef ROWITERATORS_INDEXITERATOR_HPP
#define ROWITERATORS_INDEXITERATOR_HPP

#include "RowIterator.hpp"
#include "Index.hpp"

namespace RowIterators {
    class IndexIterator : public RowIterator {
    public:
        IndexIterator(Index &index);

        RecordSchema &schema() override;

        void start() override;
        bool valid() override;

        void prev() override;
        void next() override;

        Value getField(unsigned int index) override;

    private:
        void updateTablePointer();

        Index &mIndex;
        BTree::Pointer mIndexPointer;
        BTree::Pointer mTablePointer;
    };
}
#endif