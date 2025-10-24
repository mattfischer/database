#include "RowIterators/ForeignKeyJoinIterator.hpp"

#include "Table.hpp"

namespace RowIterators {
    ForeignKeyJoinIterator::ForeignKeyJoinIterator(std::unique_ptr<RowIterator> inputIterator, unsigned int foreignKeyIndex, Table &foreignTable)
    : mInputIterator(std::move(inputIterator))
    , mForeignKeyIndex(foreignKeyIndex)
    , mForeignTable(foreignTable)
    {
        for(auto &field : mInputIterator->schema().fields) {
            mSchema.fields.push_back(field);
        }
        
        for(auto &field : mForeignTable.schema().fields) {
            mSchema.fields.push_back(field);
        }
    }

    RecordSchema &ForeignKeyJoinIterator::schema()
    {
        return mSchema;
    }

    void ForeignKeyJoinIterator::start()
    {
        mInputIterator->start();
        updateTablePointer();
    }

    bool ForeignKeyJoinIterator::valid()
    {
        return mInputIterator->valid();
    }

    void ForeignKeyJoinIterator::next()
    {
        mInputIterator->next();
        updateTablePointer();
    }

    bool ForeignKeyJoinIterator::remove()
    {
        return false;
    }

    Value ForeignKeyJoinIterator::getField(unsigned int index)
    {
        if(index < mInputIterator->schema().fields.size()) {
            return mInputIterator->getField(index);
        } else {
            void *data = mForeignTable.tree().data(mTablePointer);
            RecordReader reader(mForeignTable.schema(), data);
            return reader.readField(index - mInputIterator->schema().fields.size());
        }
    }

    void ForeignKeyJoinIterator::updateTablePointer()
    {
        if(mInputIterator->valid()) {
            Value foreignKey = mInputIterator->getField(mForeignKeyIndex);
            Table::RowId rowId = foreignKey.intValue();

            mTablePointer = mForeignTable.tree().lookup(BTree::Key(&rowId, sizeof(rowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
        }
    }
}