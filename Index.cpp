#include "Index.hpp"

class IndexKeyDefinition : public BTree::KeyDefinition {
public:
    IndexKeyDefinition(RecordSchema &schema) : mSchema(schema) {}

    virtual BTreePage::Size fixedSize() override { return 0; }
    virtual int compare(BTree::Key a, BTree::Key b) override {
        RecordReader readerA(mSchema, a.data);
        RecordReader readerB(mSchema, b.data);
        for(int i=0; i<mSchema.fields.size(); i++) {
            Value valueA = readerA.readField(i);
            Value valueB = readerB.readField(i);

            if(valueA < valueB) return -1;
            if(valueA > valueB) return 1;
        }
        return 0;
    }

    virtual void print(BTree::Key key) override {
        RecordReader reader(mSchema, key.data);
        reader.print();
    }

private:
    RecordSchema &mSchema;
};

class RowIdDataDefinition : public BTree::DataDefinition {
public:
    virtual BTreePage::Size fixedSize() override { return sizeof(Table::RowId); }

    void print(void *data)
    {
        std::cout << *reinterpret_cast<Table::RowId*>(data);
    }
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

RecordSchema &Index::keySchema()
{
    return mKeySchema;
}

void Index::add(Table::RowId rowId, RecordWriter &writer)
{
    RecordWriter keyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        keyWriter.setField(i, writer.field(mKeys[i]));
    }
    BTree::KeyValue keyValue(keyWriter.dataSize());
    keyWriter.write(keyValue.data.data());
    Pointer pointer = mTree->add(keyValue, sizeof(Table::RowId));
    void *data = mTree->data(pointer);
    std::memcpy(data, &rowId, sizeof(rowId));
}

void Index::modify(Table::RowId rowId, RecordWriter &writer)
{
    Pointer tablePointer = mTable.lookup(rowId);
    void *data = mTable.data(tablePointer);
    RecordReader reader(mTable.schema(), data);

    RecordWriter keyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        keyWriter.setField(i, reader.readField(mKeys[i]));
    }
    BTree::KeyValue keyValue(keyWriter.dataSize());
    keyWriter.write(keyValue.data.data());
    Pointer indexPointer = mTree->lookup(keyValue, BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    mTree->remove(indexPointer);

    RecordWriter newKeyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        newKeyWriter.setField(i, writer.field(mKeys[i]));
    }
    BTree::KeyValue newKeyValue(newKeyWriter.dataSize());
    newKeyWriter.write(newKeyValue.data.data());
    Pointer newPointer = mTree->add(newKeyValue, sizeof(Table::RowId));
    void *newData = mTree->data(newPointer);
    std::memcpy(newData, &rowId, sizeof(rowId));
}

void Index::remove(Table::RowId rowId, std::span<Pointer*> trackPointers)
{
    Pointer tablePointer = mTable.lookup(rowId);
    void *data = mTable.data(tablePointer);
    RecordReader reader(mTable.schema(), data);

    RecordWriter keyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        keyWriter.setField(i, reader.readField(mKeys[i]));
    }
    BTree::KeyValue keyValue(keyWriter.dataSize());
    keyWriter.write(keyValue.data.data());
    Pointer indexPointer = mTree->lookup(keyValue, BTree::SearchComparison::Equal, BTree::SearchPosition::First);
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

Index::Pointer Index::lookup(BTree::Key key, BTree::SearchComparison comparison, BTree::SearchPosition position)
{
    return mTree->lookup(key, comparison, position);
}

Table::RowId Index::rowId(Pointer pointer)
{
    return *reinterpret_cast<Table::RowId*>(mTree->data(pointer));
}

void Index::print()
{
    mTree->print();
}