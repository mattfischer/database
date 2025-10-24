#include "RowIterators/SelectIterator.hpp"

namespace RowIterators {
    SelectIterator::SelectIterator(std::unique_ptr<RowIterator> inputIterator, std::unique_ptr<Expression> predicate)
    : mInputIterator(std::move(inputIterator))
    , mPredicate(std::move(predicate))
    {
    }

    RecordSchema &SelectIterator::schema()
    {
        return mInputIterator->schema();
    }

    class IteratorContext : public Expression::Context {
    public:
        IteratorContext(RowIterator &iterator) : mIterator(iterator) {}

        Value fieldValue(unsigned int field) {
            return mIterator.getField(field);
        }

    private:
        RowIterator &mIterator;
    };

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

    Value SelectIterator::getField(unsigned int index)
    {
        return mInputIterator->getField(index);
    }

    void SelectIterator::updateIterator()
    {
        IteratorContext context(*mInputIterator);

        while(mInputIterator->valid()) {
            Value predicateValue = mPredicate->evaluate(context);
            if(predicateValue.booleanValue()) {
                break;
            }
            mInputIterator->next();
        }
    }
}