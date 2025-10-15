#include "RowIterators/SelectIterator.hpp"

namespace RowIterators {
    SelectIterator::SelectIterator(std::unique_ptr<RowIterator> inputIterator, std::unique_ptr<RowPredicate> predicate)
    : mInputIterator(std::move(inputIterator))
    , mPredicate(std::move(predicate))
    {
    }

    RecordSchema SelectIterator::schema()
    {
        return mInputIterator->schema();
    }

    void SelectIterator::start()
    {
        mInputIterator->start();
        while(mInputIterator->valid()) {
            if(mPredicate->evaluate(*mInputIterator)) {
                break;
            }
            mInputIterator->next();
        }
    }

    bool SelectIterator::valid()
    {
        return mInputIterator->valid();
    }

    void SelectIterator::prev()
    {
        mInputIterator->prev();
        while(mInputIterator->valid()) {
            if(mPredicate->evaluate(*mInputIterator)) {
                break;
            }
            mInputIterator->next();
        }
    }

    void SelectIterator::next()
    {
        mInputIterator->next();
        while(mInputIterator->valid()) {
            if(mPredicate->evaluate(*mInputIterator)) {
                break;
            }
            mInputIterator->next();
        }
    }

    Value SelectIterator::getField(unsigned int index)
    {
        return mInputIterator->getField(index);
    }
}