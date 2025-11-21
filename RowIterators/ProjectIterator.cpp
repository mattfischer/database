#include "RowIterators/ProjectIterator.hpp"

#include "Table.hpp"

namespace RowIterators {
    ProjectIterator::ProjectIterator(std::unique_ptr<RowIterator> inputIterator, std::vector<FieldDefinition> fields)
    : mInputIterator(std::move(inputIterator))
    , mFields(std::move(fields))
    {        
        for(auto &field : mFields) {
            mSchema.fields.push_back({field.expression->type(), field.name});
        }
    }

    Record::Schema &ProjectIterator::schema()
    {
        return mSchema;
    }

    void ProjectIterator::start()
    {
        mInputIterator->start();
        updateValues();
    }

    bool ProjectIterator::valid()
    {
        return mInputIterator->valid();
    }

    void ProjectIterator::next()
    {
        mInputIterator->next();
        updateValues();
    }

    bool ProjectIterator::remove()
    {
        bool result = mInputIterator->remove();
        if(result) {
            updateValues();
        }
        return result;
    }

    bool ProjectIterator::modify(const std::vector<ModifyEntry> &entries)
    {
        return false;
    }

    Value ProjectIterator::getField(unsigned int index)
    {
        return mValues[index];
    }

    void ProjectIterator::updateValues()
    {
        if(!mInputIterator->valid()) {
            return;
        }

        mValues.clear();
        for(auto &field : mFields) {
            Value value = evaluateExpression(*field.expression, *mInputIterator);
            mValues.push_back(std::move(value));
        }
    }
}