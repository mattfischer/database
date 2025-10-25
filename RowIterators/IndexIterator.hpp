#ifndef ROWITERATORS_INDEXITERATOR_HPP
#define ROWITERATORS_INDEXITERATOR_HPP

#include "RowIterator.hpp"
#include "Index.hpp"

#include <optional>

namespace RowIterators {
    class IndexIterator : public RowIterator {
    public:
        struct Limit {
            BTree::SearchComparison comparison;
            BTree::SearchPosition position;
            BTree::KeyValue key;
        };

        IndexIterator(Index &index, std::optional<Limit> startLimit, std::optional<Limit> endLimit);

        RecordSchema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;
        bool remove() override;
        bool modify(std::vector<ModifyEntry> entries) override;

        Value getField(unsigned int index) override;

    private:
        void updateTablePointer();

        Index &mIndex;
        BTree::Pointer mIndexPointer;
        Index::RowId mRowId;
        BTree::Pointer mTablePointer;

        BTree::Pointer mStartPointer;
        BTree::Pointer mEndPointer;
        std::optional<Limit> mStartLimit;
        std::optional<Limit> mEndLimit;
    };
}
#endif