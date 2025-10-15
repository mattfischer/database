#include "RowIterators/TableIterator.hpp"

namespace RowIterators {
    TableIterator::TableIterator(Table &table)
    : mTable(table)
    {
        mPointer = mTable.tree().first();
    }

    RecordSchema TableIterator::schema()
    {
        return mTable.schema();
    }

    bool TableIterator::valid()
    {
        return mTable.tree().data(mPointer);
    }

    void TableIterator::prev()
    {
        mPointer = mTable.tree().prev(mPointer);
    }

    void TableIterator::next()
    {
        mPointer = mTable.tree().next(mPointer);
    }

    Value TableIterator::getField(unsigned int index)
    {
        void *data = mTable.tree().data(mPointer);
        RecordReader reader(mTable.schema(), data);

        return reader.readField(index);
    }
}