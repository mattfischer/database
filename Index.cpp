#include "Index.hpp"

#include "Table.hpp"

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

Index::Index(Table &table, std::vector<unsigned int> keys)
: mTable(table)
, mKeys(std::move(keys))
{
    for(unsigned int index : mKeys) {
        mKeySchema.fields.push_back(table.schema().fields[index]);
    }

    PageSet &pageSet = mTable.tree().pageSet();
    Page &rootPage = pageSet.addPage();
    mTree = std::make_unique<BTree>(pageSet, rootPage.index(), std::make_unique<IndexKeyDefinition>(mKeySchema), std::make_unique<RowIdDataDefinition>());
    mTree->initialize();
}

Table &Index::table()
{
    return mTable;
}

BTree &Index::tree()
{
    return *mTree;
}

RecordSchema &Index::keySchema()
{
    return mKeySchema;
}

void Index::add(RowId rowId, RecordWriter &writer)
{
    RecordWriter keyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        keyWriter.setField(i, writer.field(mKeys[i]));
    }
    BTree::KeyValue keyValue(keyWriter.dataSize());
    keyWriter.write(keyValue.data.data());
    BTree::Pointer pointer = mTree->add(keyValue, sizeof(RowId));
    void *data = mTree->data(pointer);
    std::memcpy(data, &rowId, sizeof(rowId));
}

void Index::modify(RowId rowId, RecordWriter &writer)
{
    BTree::Pointer tablePointer = mTable.tree().lookup(BTree::Key(&rowId, sizeof(rowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    void *data = mTable.tree().data(tablePointer);
    RecordReader reader(mTable.schema(), data);

    RecordWriter keyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        keyWriter.setField(i, reader.readField(mKeys[i]));
    }
    BTree::KeyValue keyValue(keyWriter.dataSize());
    keyWriter.write(keyValue.data.data());
    BTree::Pointer indexPointer = mTree->lookup(keyValue, BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    mTree->remove(indexPointer);

    RecordWriter newKeyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        newKeyWriter.setField(i, writer.field(mKeys[i]));
    }
    BTree::KeyValue newKeyValue(newKeyWriter.dataSize());
    newKeyWriter.write(newKeyValue.data.data());
    BTree::Pointer newPointer = mTree->add(newKeyValue, sizeof(RowId));
    void *newData = mTree->data(newPointer);
    std::memcpy(newData, &rowId, sizeof(rowId));
}

void Index::remove(RowId rowId, BTree::Pointer &trackPointer)
{
    BTree::Pointer tablePointer = mTable.tree().lookup(BTree::Key(&rowId, sizeof(rowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    void *data = mTable.tree().data(tablePointer);
    RecordReader reader(mTable.schema(), data);

    RecordWriter keyWriter(mKeySchema);
    for(unsigned int i=0; i<mKeys.size(); i++) {
        keyWriter.setField(i, reader.readField(mKeys[i]));
    }
    BTree::KeyValue keyValue(keyWriter.dataSize());
    keyWriter.write(keyValue.data.data());
    BTree::Pointer indexPointer = mTree->lookup(keyValue, BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    if(indexPointer == trackPointer) {
        trackPointer = mTree->remove(indexPointer);
    } else {
        mTree->remove(indexPointer);
    }
}

void Index::print()
{
    mTree->print();
}