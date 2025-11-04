#include "RowIterators/TableIterator.hpp"

namespace RowIterators {
    TableIterator::TableIterator(Table &table)
    : mTable(table)
    {
    }

    Record::Schema &TableIterator::schema()
    {
        return mTable.schema();
    }

    void TableIterator::start()
    {
        mPointer = mTable.first();
    }

    bool TableIterator::valid()
    {
        return mPointer.valid();
    }

    void TableIterator::next()
    {
        mTable.moveNext(mPointer);
    }

    bool TableIterator::remove()
    {
        Table::Pointer *pointers[] = {&mPointer};
        mTable.removeRow(mPointer, pointers);
        return true;
    }

    bool TableIterator::modify(std::vector<ModifyEntry> entries)
    {
        Record::Writer writer(mTable.schema());
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
        Record::Reader reader(mTable.schema(), data);

        return reader.readField(index);
    }
}