#include "RowIterators/TableIterator.hpp"

namespace RowIterators {
    TableIterator::TableIterator(Table &table)
    : mTable(table)
    {
    }

    RecordSchema &TableIterator::schema()
    {
        return mTable.schema();
    }

    void TableIterator::start()
    {
        mPointer = mTable.tree().first();
    }

    bool TableIterator::valid()
    {
        return mTable.tree().data(mPointer);
    }

    void TableIterator::next()
    {
        mPointer = mTable.tree().next(mPointer);
    }

    bool TableIterator::remove()
    {
        mPointer = mTable.tree().remove(mPointer);
        return true;
    }

    Value TableIterator::getField(unsigned int index)
    {
        void *data = mTable.tree().data(mPointer);
        RecordReader reader(mTable.schema(), data);

        return reader.readField(index);
    }
}