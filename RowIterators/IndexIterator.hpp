#ifndef ROWITERATORS_INDEXITERATOR_HPP
#define ROWITERATORS_INDEXITERATOR_HPP

#include "RowIterator.hpp"
#include "Index.hpp"

#include <optional>

namespace RowIterators {
    class IndexIterator : public RowIterator {
    public:
        IndexIterator(Index &index, std::optional<Index::Limit> startLimit, std::optional<Index::Limit> endLimit);

        Record::Schema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;
        bool remove() override;
        bool modify(const std::vector<ModifyEntry> &entries) override;

        Value getField(unsigned int index) override;

    private:
        void updateTablePointer();
    
        Index &mIndex;
        Index::Pointer mIndexPointer;
        Table::RowId mRowId;
        Table::Pointer mTablePointer;

        Index::Pointer mStartPointer;
        Index::Pointer mEndPointer;
        std::optional<Index::Limit> mStartLimit;
        std::optional<Index::Limit> mEndLimit;
    };
}
#endif