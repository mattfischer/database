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
            mStartPointer = mIndex.lookup(mStartLimit->key, mStartLimit->comparison, mStartLimit->position);
        } else {
            mStartPointer = mIndex.first();
        }

        if(mEndLimit) {
            mEndPointer = mIndex.lookup(mEndLimit->key, mEndLimit->comparison, mEndLimit->position);
        } else {
            mEndPointer = mIndex.last();
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
            mIndex.moveNext(mIndexPointer);
            updateTablePointer();
        }
    }

    bool IndexIterator::remove()
    {
        Index::Pointer* pointers[] = {&mIndexPointer, &mStartPointer, &mEndPointer};
        mIndex.table().removeRow(mRowId, pointers);
        updateTablePointer();

        return true;
    }

    bool IndexIterator::modify(std::vector<ModifyEntry> entries)
    {
        return false;
    }

    Value IndexIterator::getField(unsigned int index)
    {
        void *data = mIndex.table().data(mTablePointer);

        RecordReader reader(mIndex.table().schema(), data);
        return reader.readField(index);
    }

    void IndexIterator::updateTablePointer()
    {
        if(mIndexPointer.valid()) {
            mRowId = mIndex.rowId(mIndexPointer);
            mTablePointer = mIndex.table().lookup(mRowId);
        }
    }
}