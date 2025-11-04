#include "Table.hpp"

#include "Index.hpp"

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
    RowDataDefinition(Record::Schema &schema) : mSchema(schema) {}
    BTreePage::Size fixedSize() override { return 0; }

    void print(void *data) override
    {
        Record::Reader reader(mSchema, data);
        reader.print();
    }

private:
    Record::Schema &mSchema;
};

Table::Table(Page &rootPage, Record::Schema schema)
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

Record::Schema &Table::schema()
{
    return mSchema;
}

std::vector<Index*> &Table::indices()
{
    return mIndices;
}

Table::RowId Table::addRow(Record::Writer &writer)
{
    RowId rowId = mNextRowId;
    Pointer pointer = mTree.add(BTree::Key(&rowId, sizeof(rowId)), writer.dataSize());
    void *data = mTree.data(pointer);
    writer.write(data);

    for(auto &index : mIndices) {
        index->add(rowId, writer);
    }

    mNextRowId++;

    return rowId;
}

void Table::modifyRow(RowId rowId, Record::Writer &writer)
{
    for(auto &index : mIndices) {
        index->modify(rowId, writer);
    }

    Pointer pointer = mTree.lookup(BTree::Key(&rowId, sizeof(rowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    mTree.resize(pointer, writer.dataSize());
    void *data = mTree.data(pointer);
    writer.write(data);
}

void Table::modifyRow(Pointer pointer, Record::Writer &writer)
{
    RowId rowId = getRowId(pointer);
    for(auto &index : mIndices) {
        index->modify(rowId, writer);
    }

    mTree.resize(pointer, writer.dataSize());
    void *data = mTree.data(pointer);
    writer.write(data);
}

void Table::removeRow(RowId rowId, std::span<Pointer*> trackPointers)
{
    for(auto &index : mIndices) {
        index->remove(rowId, trackPointers);
    }

    Pointer pointer = mTree.lookup(BTree::Key(&rowId, sizeof(rowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
    mTree.remove(pointer, trackPointers);
}

void Table::removeRow(Pointer pointer, std::span<Pointer*> trackPointers)
{
    mTree.remove(pointer, trackPointers);
}

Table::Pointer Table::first()
{
    return mTree.first();
}

Table::Pointer Table::last()
{
    return mTree.last();
}

bool Table::moveNext(Pointer &pointer)
{
    return mTree.moveNext(pointer);
}

bool Table::movePrev(Pointer &pointer)
{
    return mTree.movePrev(pointer);
}

Table::Pointer Table::lookup(RowId rowId)
{
    return mTree.lookup(BTree::Key(&rowId, sizeof(rowId)), BTree::SearchComparison::Equal, BTree::SearchPosition::First);
}

Table::RowId Table::getRowId(Pointer pointer)
{
    return *reinterpret_cast<RowId*>(mTree.key(pointer));
}

void *Table::data(Pointer pointer)
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
