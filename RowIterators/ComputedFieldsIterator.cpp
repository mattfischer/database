#include "RowIterators/ComputedFieldsIterator.hpp"

#include "Table.hpp"

namespace RowIterators {
    ComputedFieldsIterator::ComputedFieldsIterator(RowIterator &inputIterator, std::vector<ComputedField> computedFields)
    : mInputIterator(inputIterator)
    , mComputedFields(std::move(computedFields))
    {
        for(auto &field : mInputIterator.schema().fields) {
            mSchema.fields.push_back(field);
        }
        
        for(auto &computedField : mComputedFields) {
            mSchema.fields.push_back({computedField.type, computedField.name});
        }
    }

    RecordSchema &ComputedFieldsIterator::schema()
    {
        return mSchema;
    }

    void ComputedFieldsIterator::start()
    {
        mInputIterator.start();
        updateComputedFields();
    }

    bool ComputedFieldsIterator::valid()
    {
        return mInputIterator.valid();
    }

    void ComputedFieldsIterator::next()
    {
        mInputIterator.next();
        updateComputedFields();
    }

    Value ComputedFieldsIterator::getField(unsigned int index)
    {
        if(index < mInputIterator.schema().fields.size()) {
            return mInputIterator.getField(index);
        } else {
            return mComputedValues[index - mInputIterator.schema().fields.size()];
        }
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

    void ComputedFieldsIterator::updateComputedFields()
    {
        if(!mInputIterator.valid()) {
            return;
        }

        IteratorContext context(mInputIterator);
        mComputedValues.clear();
        for(auto &computedField : mComputedFields) {
            Value computedValue = computedField.expression->evaluate(context);
            mComputedValues.push_back(std::move(computedValue));
        }
    }
}