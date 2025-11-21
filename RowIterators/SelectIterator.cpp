#include "RowIterators/SelectIterator.hpp"

namespace RowIterators {
    SelectIterator::SelectIterator(std::unique_ptr<RowIterator> inputIterator, std::unique_ptr<Expression> predicate)
    : mInputIterator(std::move(inputIterator))
    , mPredicate(std::move(predicate))
    {
    }

    Record::Schema &SelectIterator::schema()
    {
        return mInputIterator->schema();
    }

    void SelectIterator::start()
    {
        mInputIterator->start();
        updateIterator();        
    }

    bool SelectIterator::valid()
    {
        return mInputIterator->valid();
    }

    void SelectIterator::next()
    {
        mInputIterator->next();
        updateIterator();
    }

    bool SelectIterator::remove()
    {
        bool result = mInputIterator->remove();
        if(result) {
            updateIterator();
        }
        return result;
    }

    bool SelectIterator::modify(const std::vector<ModifyEntry> &entries)
    {
        return mInputIterator->modify(entries);
    }

    Value SelectIterator::getField(unsigned int index)
    {
        return mInputIterator->getField(index);
    }

    void SelectIterator::updateIterator()
    {
        while(mInputIterator->valid()) {
            Value predicateValue = evaluateExpression(*mPredicate, *mInputIterator);
            if(predicateValue.booleanValue()) {
                break;
            }
            mInputIterator->next();
        }
    }
}