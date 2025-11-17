#include "RowIterators/ExtendedProjectIterator.hpp"

#include "Table.hpp"

namespace RowIterators {
    ExtendedProjectIterator::ExtendedProjectIterator(std::unique_ptr<RowIterator> inputIterator, std::vector<FieldDefinition> fields)
    : mInputIterator(std::move(inputIterator))
    , mFields(std::move(fields))
    {
        for(auto &field : mInputIterator->schema().fields) {
            mSchema.fields.push_back(field);
        }
        
        for(auto &field : mFields) {
            mSchema.fields.push_back({field.type, field.name});
        }
    }

    Record::Schema &ExtendedProjectIterator::schema()
    {
        return mSchema;
    }

    void ExtendedProjectIterator::start()
    {
        mInputIterator->start();
        updateValues();
    }

    bool ExtendedProjectIterator::valid()
    {
        return mInputIterator->valid();
    }

    void ExtendedProjectIterator::next()
    {
        mInputIterator->next();
        updateValues();
    }

    bool ExtendedProjectIterator::remove()
    {
        bool result = mInputIterator->remove();
        if(result) {
            updateValues();
        }
        return result;
    }

    bool ExtendedProjectIterator::modify(const std::vector<ModifyEntry> &entries)
    {
        return false;
    }

    Value ExtendedProjectIterator::getField(unsigned int index)
    {
        if(index < mInputIterator->schema().fields.size()) {
            return mInputIterator->getField(index);
        } else {
            return mValues[index - mInputIterator->schema().fields.size()];
        }
    }

    class IteratorEvaluateContext : public Expression::EvaluateContext {
    public:
        IteratorEvaluateContext(RowIterator &iterator) : mIterator(iterator) {}

        Value fieldValue(unsigned int field) {
            return mIterator.getField(field);
        }

    private:
        RowIterator &mIterator;
    };

    void ExtendedProjectIterator::updateValues()
    {
        if(!mInputIterator->valid()) {
            return;
        }

        IteratorEvaluateContext context(*mInputIterator);
        mValues.clear();
        for(auto &field : mFields) {
            Value value = field.expression->evaluate(context);
            mValues.push_back(std::move(value));
        }
    }
}