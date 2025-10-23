#include "RowIterators/IndexIterator.hpp"

#include "Table.hpp"

namespace RowIterators {
    IndexIterator::IndexIterator(Index &index)
    : mIndex(index)
    {
    }

    RecordSchema &IndexIterator::schema()
    {
        return mIndex.table().schema();
    }

    void IndexIterator::start()
    {
        mIndexPointer = mIndex.tree().first();
        updateTablePointer();
    }

    bool IndexIterator::valid()
    {
        return mIndex.tree().data(mIndexPointer);
    }

    void IndexIterator::next()
    {
        mIndexPointer = mIndex.tree().next(mIndexPointer);
        updateTablePointer();
    }

    Value IndexIterator::getField(unsigned int index)
    {
        void *data = mIndex.table().tree().data(mTablePointer);

        RecordReader reader(mIndex.table().schema(), data);
        return reader.readField(index);
    }

    void IndexIterator::updateTablePointer()
    {
        void *data = mIndex.tree().data(mIndexPointer);
        if(data) {
            Table::RowId rowId = *reinterpret_cast<Table::RowId*>(data);
        
            mTablePointer = mIndex.table().tree().lookup(BTree::Key(&rowId, sizeof(rowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
        }
    }
}