#include "Index.hpp"

class IndexKeyDefinition : public BTree::KeyDefinition {
public:
    IndexKeyDefinition(Record::Schema &schema) : mSchema(schema) {}

    virtual BTreePage::Size fixedSize() override { return 0; }
    virtual int compare(BTree::Key a, BTree::Key b) override {
        Record::Reader readerA(mSchema, a.data);
        Record::Reader readerB(mSchema, b.data);
        for(int i=0; i<mSchema.fields.size(); i++) {
            Value valueA = readerA.readField(i);
            Value valueB = readerB.readField(i);

            if(valueA < valueB) return -1;
            if(valueA > valueB) return 1;
        }
        return 0;
    }

    virtual void print(BTree::Key key) override {
        Record::Reader reader(mSchema, key.data);
        reader.print();
    }

private:
    Record::Schema &mSchema;
};

class RowIdDataDefinition : public BTree::DataDefinition {
public:
    virtual BTreePage::Size fixedSize() override { return sizeof(Table::RowId); }

    void print(void *data)
    {
        std::cout << *reinterpret_cast<Table::RowId*>(data);
    }
};

class RecordKey {
public:
    RecordKey(Record::Writer &writer) {
        mData.resize(writer.dataSize());
        writer.write(mData.data());
    }

    operator BTree::Key() {
        return BTree::Key(mData.data(), mData.size());
    }

private:
    std::vector<uint8_t> mData;
};

Index::Index(Page &rootPage, Table &table, std::vector<unsigned int> keys)
: mTable(table)
, mKeys(std::move(keys))
{
    for(unsigned int index : mKeys) {
        mKeySchema.fields.push_back(table.schema().fields[index]);
    }

    mTree = std::make_unique<BTree>(rootPage.pageSet(), rootPage.index(), std::make_unique<IndexKeyDefinition>(mKeySchema), std::make_unique<RowIdDataDefinition>());
    mTree->initialize();
}

Table &Index::table()
{
    return mTable;
}

Record::Schema &Index::keySchema()
{
    return mKeySchema;
}

void Index::add(Table::RowId rowId, Record::Writer &writer)
{
    Record::Writer keyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        keyWriter.setField(i, writer.field(mKeys[i]));
    }
    RecordKey key(keyWriter);
    Pointer pointer = mTree->add(key, sizeof(Table::RowId));
    void *data = mTree->data(pointer);
    std::memcpy(data, &rowId, sizeof(rowId));
}

void Index::modify(Table::RowId rowId, Record::Writer &writer)
{
    Pointer tablePointer = mTable.lookup(rowId);
    void *data = mTable.data(tablePointer);
    Record::Reader reader(mTable.schema(), data);

    Record::Writer keyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        keyWriter.setField(i, reader.readField(mKeys[i]));
    }
    RecordKey key(keyWriter);
    Pointer indexPointer = mTree->lookup(key, BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    mTree->remove(indexPointer);

    Record::Writer newKeyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        newKeyWriter.setField(i, writer.field(mKeys[i]));
    }
    RecordKey newKey(newKeyWriter);
    Pointer newPointer = mTree->add(newKey, sizeof(Table::RowId));
    void *newData = mTree->data(newPointer);
    std::memcpy(newData, &rowId, sizeof(rowId));
}

void Index::remove(Table::RowId rowId, std::span<Pointer*> trackPointers)
{
    Pointer tablePointer = mTable.lookup(rowId);
    void *data = mTable.data(tablePointer);
    Record::Reader reader(mTable.schema(), data);

    Record::Writer keyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        keyWriter.setField(i, reader.readField(mKeys[i]));
    }
    RecordKey key(keyWriter);
    Pointer indexPointer = mTree->lookup(key, BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    mTree->remove(indexPointer, trackPointers);
}

Index::Pointer Index::first()
{
    return mTree->first();
}

Index::Pointer Index::last()
{
    return mTree->last();
}

bool Index::moveNext(Pointer &pointer)
{
    return mTree->moveNext(pointer);
}

bool Index::movePrev(Pointer &pointer)
{
    return mTree->movePrev(pointer);
}

Table::RowId Index::rowId(Pointer pointer)
{
    return *reinterpret_cast<Table::RowId*>(mTree->data(pointer));
}

void Index::print()
{
    mTree->print();
}

Index::Pointer Index::lookup(Limit &limit)
{
    BTree::KeyComparator comparator = [&](BTree::Key a, BTree::Key b) {
        return partialKeyCompare(a, b, limit.values.size());
    };
    Record::Schema schema;
    for(int i=0; i<limit.values.size(); i++) {
        schema.fields.push_back(keySchema().fields[i]);
    }
    Record::Writer keyWriter(schema);
    for(int i=0; i<limit.values.size(); i++) {
        keyWriter.setField(i, limit.values[i]);
    }
    RecordKey key(keyWriter);
    return mTree->lookup(key, comparator, limit.comparison, limit.position);
}

int Index::partialKeyCompare(BTree::Key a, BTree::Key b, int numFields)
{
    Record::Reader readerA(keySchema(), a.data);
    Record::Reader readerB(keySchema(), b.data);
    for(int i=0; i<numFields; i++) {
        Value valueA = readerA.readField(i);
        Value valueB = readerB.readField(i);

        if(valueA < valueB) return -1;
        if(valueA > valueB) return 1;
    }
    return 0;
}
