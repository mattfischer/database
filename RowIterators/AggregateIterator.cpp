#include "RowIterators/AggregateIterator.hpp"

#include "Record.hpp"

#include <algorithm>

namespace RowIterators {
    AggregateIterator::AggregateIterator(std::unique_ptr<RowIterator> inputIterator, Operation operation, unsigned int field, unsigned int groupField)
    : mInputIterator(std::move(inputIterator))
    {
        mOperation = operation;
        mField = field;
        mGroupField = groupField;

        if(mGroupField != kFieldNone) {
            mSchema.fields.push_back(mInputIterator->schema().fields[mGroupField]);
        }

        switch(mOperation) {
            case Min:
                mSchema.fields.push_back({mInputIterator->schema().fields[mField].type, "min"});
                break;
            case Average:
                mSchema.fields.push_back({Value::Type::Float, "average"});
                break;
            case Sum:
                mSchema.fields.push_back({mInputIterator->schema().fields[mField].type, "sum"});
                break;
            case Max:
                mSchema.fields.push_back({mInputIterator->schema().fields[mField].type, "max"});
                break;
            case Count:
                mSchema.fields.push_back({Value::Type::Int, "count"});
                break;
        }
    }

    Record::Schema &AggregateIterator::schema()
    {
        return mSchema;
    }

    void AggregateIterator::start()
    {
        mInputIterator->start();
        update();
    }

    bool AggregateIterator::valid()
    {
        return mValid;
    }

    void AggregateIterator::next()
    {
        if(mValid) {
            update();
        }
    }

    bool AggregateIterator::remove()
    {
        return false;
    }

    bool AggregateIterator::modify(std::vector<ModifyEntry> entries)
    {
        return false;
    }

    Value AggregateIterator::getField(unsigned int index)
    {
        if(mGroupField == kFieldNone) {
            return mValue;
        } else {
            if(index == 0) {
                return mGroupValue;
            } else {
                return mValue;
            }
        }
    }

    void AggregateIterator::update()
    {
        if(!mInputIterator->valid()) {
            mValid = false;
            return;
        }

        int count = 0;
        Value value;
        if(mInputIterator->valid() && mGroupField != kFieldNone) {
            mGroupValue = mInputIterator->getField(mGroupField);
        }
        while(mInputIterator->valid()) {
            if(mGroupField != kFieldNone) {
                Value groupValue = mInputIterator->getField(mGroupField);
                if(groupValue != mGroupValue) {
                    break;
                }
            }

            Value newValue = mInputIterator->getField(mField);
            switch(mOperation) {
                case Min:
                    if(count == 0 || newValue < value) value = newValue;
                    break;
                case Average:
                case Sum:
                    if(count == 0) value = newValue; else value = value + newValue;
                    break;
                case Max:
                    if(count == 0 || newValue > value) value = newValue;
                    break;
            }

            count++;
            mInputIterator->next();
        }

        switch(mOperation) {
            case Min:
            case Max:
            case Sum:
                mValue = value;
                break;
            case Average:
            {
                float sum = 0;
                switch(mInputIterator->schema().fields[mField].type) {
                    case Value::Type::Int:
                        sum = (float)value.intValue();
                        break;
                    case Value::Type::Float:
                        sum = value.floatValue();
                        break;
                }
                mValue = Value(sum / count);
                break;
            }
            case Count:
                mValue = Value(count);
                break;
        }

        mValid = true;
    }
}