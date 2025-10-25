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
        return mPointer.valid();
    }

    void TableIterator::next()
    {
        mPointer = mTable.tree().next(mPointer);
    }

    bool TableIterator::remove()
    {
        mTable.removeRow(mPointer);
        return true;
    }

    bool TableIterator::modify(std::vector<ModifyEntry> entries)
    {
        RecordWriter writer(mTable.schema());
        for(int i=0; i<mTable.schema().fields.size(); i++) {
            writer.setField(i, getField(i));
        }

        for(auto &entry : entries) {
            writer.setField(entry.field, entry.value);
        }

        mTable.modifyRow(mPointer, writer);

        return true;
    }

    Value TableIterator::getField(unsigned int index)
    {
        void *data = mTable.data(mPointer);
        RecordReader reader(mTable.schema(), data);

        return reader.readField(index);
    }
}