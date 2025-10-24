#ifndef ROWITERATORS_AGGREGATEITERATOR_HPP
#define ROWITERATORS_AGGREGATEITERATOR_HPP

#include "RowIterator.hpp"
#include "BTree.hpp"

#include <memory>

namespace RowIterators {
    class AggregateIterator : public RowIterator {
    public:
        static const unsigned int kFieldNone = UINT32_MAX;

        enum Operation {
            Min,
            Average,
            Sum,
            Max,
            Count
        };

        AggregateIterator(std::unique_ptr<RowIterator> inputIterator, Operation operation, unsigned int field, unsigned int groupField);

        RecordSchema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;
        bool remove() override;

        Value getField(unsigned int index) override;

    private:
        void update();

        std::unique_ptr<RowIterator> mInputIterator;
        Operation mOperation;
        unsigned int mField;
        unsigned int mGroupField;

        RecordSchema mSchema;
        Value mGroupValue;
        bool mValid;
        Value mValue;
        int mCount;
    };
}
#endif