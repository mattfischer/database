#include "RowIterators/IndexIterator.hpp"

#include "Table.hpp"

namespace RowIterators {
    IndexIterator::IndexIterator(Index &index, std::optional<Index::Limit> startLimit, std::optional<Index::Limit> endLimit)
    : mIndex(index)
    , mStartLimit(std::move(startLimit))
    , mEndLimit(std::move(endLimit))
    {
    }

    Record::Schema &IndexIterator::schema()
    {
        return mIndex.table().schema();
    }

    void IndexIterator::start()
    {
        mStartPointer = mStartLimit ? mIndex.lookup(*mStartLimit) : mIndex.first();
        mEndPointer = mEndLimit ? mIndex.lookup(*mEndLimit) : mIndex.last();

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

    bool IndexIterator::modify(const std::vector<ModifyEntry> &entries)
    {
        return false;
    }

    Value IndexIterator::getField(unsigned int index)
    {
        void *data = mIndex.table().data(mTablePointer);

        Record::Reader reader(mIndex.table().schema(), data);
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