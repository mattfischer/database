#include "RowIterators/IndexIterator.hpp"

#include "Table.hpp"

namespace RowIterators {
    IndexIterator::IndexIterator(Index &index, std::optional<Limit> startLimit, std::optional<Limit> endLimit)
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
        if(mStartLimit) {
            mStartPointer = lookupLimit(*mStartLimit);
        } else {
            mStartPointer = mIndex.first();
        }

        if(mEndLimit) {
            mEndPointer = lookupLimit(*mEndLimit);
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

    int IndexIterator::partialKeyCompare(BTree::Key a, BTree::Key b, int numFields)
    {
        Record::Reader readerA(mIndex.keySchema(), a.data);
        Record::Reader readerB(mIndex.keySchema(), b.data);
        for(int i=0; i<numFields; i++) {
            Value valueA = readerA.readField(i);
            Value valueB = readerB.readField(i);

            if(valueA < valueB) return -1;
            if(valueA > valueB) return 1;
        }
        return 0;
    }

    void IndexIterator::updateTablePointer()
    {
        if(mIndexPointer.valid()) {
            mRowId = mIndex.rowId(mIndexPointer);
            mTablePointer = mIndex.table().lookup(mRowId);
        }
    }

    BTree::Pointer IndexIterator::lookupLimit(Limit &limit)
    {
        BTree::KeyComparator comparator = [&](BTree::Key a, BTree::Key b) {
            return partialKeyCompare(a, b, limit.values.size());
        };
        Record::Schema keySchema;
        for(int i=0; i<limit.values.size(); i++) {
            keySchema.fields.push_back(schema().fields[i]);
        }
        Record::Writer keyWriter(keySchema);
        for(int i=0; i<limit.values.size(); i++) {
            keyWriter.setField(i, limit.values[i]);
        }
        Index::RecordKey key(keyWriter);
        return mIndex.lookup(key, comparator, limit.comparison, limit.position);
    }
}