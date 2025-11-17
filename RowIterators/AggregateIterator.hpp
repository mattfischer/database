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

        Record::Schema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;
        bool remove() override;
        bool modify(const std::vector<ModifyEntry> &entries) override;

        Value getField(unsigned int index) override;

    private:
        void update();

        std::unique_ptr<RowIterator> mInputIterator;
        Operation mOperation;
        unsigned int mField;
        unsigned int mGroupField;

        Record::Schema mSchema;
        Value mGroupValue;
        bool mValid;
        Value mValue;
    };
}
#endif