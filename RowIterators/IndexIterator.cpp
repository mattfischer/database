#include "RowIterators/IndexIterator.hpp"

#include "Table.hpp"

namespace RowIterators {
    IndexIterator::IndexIterator(Index &index, std::optional<Limit> startLimit, std::optional<Limit> endLimit)
    : mIndex(index)
    , mStartLimit(std::move(startLimit))
    , mEndLimit(std::move(endLimit))
    {
    }

    RecordSchema &IndexIterator::schema()
    {
        return mIndex.table().schema();
    }

    void IndexIterator::start()
    {
        if(mStartLimit) {
            mStartPointer = mIndex.tree().lookup(mStartLimit->key, mStartLimit->comparison, mStartLimit->position);
        } else {
            mStartPointer = mIndex.tree().first();
        }

        if(mEndLimit) {
            mEndPointer = mIndex.tree().lookup(mEndLimit->key, mEndLimit->comparison, mEndLimit->position);
        } else {
            mEndPointer = mIndex.tree().last();
        }

        mIndexPointer = mStartPointer;
        updateTablePointer();
    }

    bool IndexIterator::valid()
    {
        return mIndexPointer.valid();
    }

    void IndexIterator::next()
    {
        if(mIndexPointer == mEndPointer) {
            mIndexPointer = {Page::kInvalidIndex, 0};
        } else {
            mIndexPointer = mIndex.tree().next(mIndexPointer);
            updateTablePointer();
        }
    }

    bool IndexIterator::remove()
    {
        mIndex.table().removeRow(mRowId, mIndexPointer);
        updateTablePointer();

        return true;
    }

    bool IndexIterator::modify(std::vector<ModifyEntry> entries)
    {
        return false;
    }

    Value IndexIterator::getField(unsigned int index)
    {
        void *data = mIndex.table().tree().data(mTablePointer);

        RecordReader reader(mIndex.table().schema(), data);
        return reader.readField(index);
    }

    void IndexIterator::updateTablePointer()
    {
        if(mIndexPointer.valid()) {
            void *data = mIndex.tree().data(mIndexPointer);
            mRowId = *reinterpret_cast<Table::RowId*>(data);
        
            mTablePointer = mIndex.table().tree().lookup(BTree::Key(&mRowId, sizeof(mRowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
        }
    }
}