#ifndef ROWITERATORS_SELECTITERATOR_HPP
#define ROWITERATORS_SELECTITERATOR_HPP

#include "RowIterator.hpp"
#include "Expression.hpp"

#include <memory>

namespace RowIterators {
    class SelectIterator : public RowIterator {
    public:
        SelectIterator(std::unique_ptr<RowIterator> inputIterator, std::unique_ptr<Expression> predicate);

        RecordSchema &schema() override;

        void start() override;
        bool valid() override;
        void next() override;

        Value getField(unsigned int index) override;

    private:
        void updateIterator();

        std::unique_ptr<RowIterator> mInputIterator;
        std::unique_ptr<Expression> mPredicate;
    };
}

#endif