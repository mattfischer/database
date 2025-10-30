#include "Table.hpp"

class RowIdKeyDefinition : public BTree::KeyDefinition {
public:
    virtual BTreePage::Size fixedSize() override { return sizeof(Table::RowId); }
    virtual int compare(BTree::Key a, BTree::Key b) override {
        Table::RowId ar = *reinterpret_cast<Table::RowId*>(a.data);
        Table::RowId br = *reinterpret_cast<Table::RowId*>(b.data);
        if(ar < br) return -1;
        if(ar == br) return 0;
        return 1;
    }
    virtual void print(BTree::Key key) override {
        std::cout << *reinterpret_cast<Table::RowId*>(key.data);
    }
};

class RowDataDefinition : public BTree::DataDefinition {
public:
    RowDataDefinition(RecordSchema &schema) : mSchema(schema) {}
    BTreePage::Size fixedSize() override { return 0; }

    void print(void *data) override
    {
        RecordReader reader(mSchema, data);
        reader.print();
    }

private:
    RecordSchema &mSchema;
};

Table::Table(Page &rootPage, RecordSchema schema)
: mPageSet(rootPage.pageSet())
, mSchema(std::move(schema))
, mTree(mPageSet, rootPage.index(), std::make_unique<RowIdKeyDefinition>(), std::make_unique<RowDataDefinition>(mSchema))
{
}

void Table::initialize()
{
    mTree.initialize();
    mNextRowId = 1;
}

RecordSchema &Table::schema()
{
    return mSchema;
}

BTree &Table::tree()
{
    return mTree;
}

std::vector<Index*> &Table::indices()
{
    return mIndices;
}

Table::RowId Table::addRow(RecordWriter &writer)
{
    RowId rowId = mNextRowId;
    BTree::Pointer pointer = mTree.add(BTree::Key(&rowId, sizeof(rowId)), writer.dataSize());
    void *data = mTree.data(pointer);
    writer.write(data);

    for(auto &index : mIndices) {
        index->add(rowId, writer);
    }

    mNextRowId++;

    return rowId;
}

void Table::modifyRow(RowId rowId, RecordWriter &writer)
{
    for(auto &index : mIndices) {
        index->modify(rowId, writer);
    }

    BTree::Pointer pointer = mTree.lookup(BTree::Key(&rowId, sizeof(rowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    mTree.resize(pointer, writer.dataSize());
    void *data = mTree.data(pointer);
    writer.write(data);
}

void Table::modifyRow(BTree::Pointer pointer, RecordWriter &writer)
{
    RowId rowId = getRowId(pointer);
    for(auto &index : mIndices) {
        index->modify(rowId, writer);
    }

    mTree.resize(pointer, writer.dataSize());
    void *data = mTree.data(pointer);
    writer.write(data);
}

void Table::removeRow(RowId rowId, std::span<BTree::Pointer*> trackPointers)
{
    for(auto &index : mIndices) {
        index->remove(rowId, trackPointers);
    }

    BTree::Pointer pointer = mTree.lookup(BTree::Key(&rowId, sizeof(rowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    mTree.remove(pointer, trackPointers);
}

void Table::removeRow(BTree::Pointer pointer, std::span<BTree::Pointer*> trackPointers)
{
    mTree.remove(pointer, trackPointers);
}

Table::RowId Table::getRowId(BTree::Pointer pointer)
{
    return *reinterpret_cast<RowId*>(mTree.key(pointer));
}

BTree::Pointer Table::lookup(RowId rowId)
{
    BTree::Pointer pointer = mTree.lookup(BTree::Key(&rowId, sizeof(rowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    return pointer;
}

void *Table::data(BTree::Pointer pointer)
{
    if(pointer.valid()) {
        return mTree.data(pointer);
    } else {
        return nullptr;
    }
}

void Table::addIndex(Index &index)
{
    mIndices.push_back(&index);
}

void Table::print()
{
    mTree.print();
}
