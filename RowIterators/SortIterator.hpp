#ifndef ROWITERATORS_SORTITERATOR_HPP
#define ROWITERATORS_SORTITERATOR_HPP

#include "RowIterator.hpp"

#include <memory>

namespace RowIterators {
    class SortIterator : public RowIterator {
    public:
        SortIterator(std::unique_ptr<RowIterator> inputIterator, unsigned int sortField);

        RecordSchema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;
        bool remove() override;

        Value getField(unsigned int index) override;

    private:
        std::unique_ptr<RowIterator> mInputIterator;
        unsigned int mSortField;
        std::vector<uint8_t> mData;
        std::vector<unsigned int> mOffsets;
        unsigned int mRow;
    };
}
#endif