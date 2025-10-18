#include "RowIterators/ProjectIterator.hpp"

namespace RowIterators {
    ProjectIterator::ProjectIterator(std::unique_ptr<RowIterator> inputIterator, std::vector<FieldDefinition> fields)
    : mInputIterator(std::move(inputIterator))
    , mFields(std::move(fields))
    {
        for(auto &field : mFields) {
            mSchema.fields.push_back({mInputIterator->schema().fields[field.index].type, field.name});
        }
    }

    RecordSchema &ProjectIterator::schema()
    {
        return mSchema;
    }

    void ProjectIterator::start()
    {
        mInputIterator->start();
    }

    bool ProjectIterator::valid()
    {
        return mInputIterator->valid();
    }

    void ProjectIterator::next()
    {
        mInputIterator->next();
    }

    Value ProjectIterator::getField(unsigned int index)
    {
        return mInputIterator->getField(mFields[index].index);
    }
}