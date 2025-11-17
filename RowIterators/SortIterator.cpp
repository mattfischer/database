#include "RowIterators/SortIterator.hpp"

#include "Record.hpp"

#include <algorithm>

namespace RowIterators {
    SortIterator::SortIterator(std::unique_ptr<RowIterator> inputIterator, unsigned int sortField)
    : mInputIterator(std::move(inputIterator))
    {
        mSortField = sortField;
    }

    Record::Schema &SortIterator::schema()
    {
        return mInputIterator->schema();
    }

    void SortIterator::start()
    {
        mInputIterator->start();

        while(mInputIterator->valid()) {
            Record::Writer writer(mInputIterator->schema());
            for(unsigned int i=0; i<mInputIterator->schema().fields.size(); i++) {
                writer.setField(i, mInputIterator->getField(i));
            }
            
            unsigned int startOffset = mData.size();
            mOffsets.push_back(startOffset);
            mData.resize(mData.size() + writer.dataSize());
            writer.write(mData.data() + startOffset);

            mInputIterator->next();
        }

        auto cmp = [&](unsigned int a, unsigned int b) {
            Record::Reader readerA(mInputIterator->schema(), mData.data() + a);
            Record::Reader readerB(mInputIterator->schema(), mData.data() + b);
            Value valueA = readerA.readField(mSortField);
            Value valueB = readerB.readField(mSortField);

            return valueA < valueB;
        };

        std::sort(mOffsets.begin(), mOffsets.end(), cmp);
        mRow = 0;
    }

    bool SortIterator::valid()
    {
        return mRow < mOffsets.size();
    }

    void SortIterator::next()
    {
        mRow++;
    }

    bool SortIterator::remove()
    {
        return false;
    }

    bool SortIterator::modify(const std::vector<ModifyEntry> &entries)
    {
        return false;
    }

    Value SortIterator::getField(unsigned int index)
    {
        Record::Reader reader(mInputIterator->schema(), mData.data() + mOffsets[mRow]);

        return reader.readField(index);
    }
}