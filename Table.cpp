#include "Table.hpp"

class RowIdKeyDefinition : public TreePage::KeyDefinition {
public:
    virtual TreePage::Size fixedSize() override { return sizeof(Table::RowId); }
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

Table::Table(Page &rootPage, RecordSchema schema)
: mPageSet(rootPage.pageSet())
, mSchema(std::move(schema))
, mTree(mPageSet, rootPage.index(), std::make_unique<RowIdKeyDefinition>())
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

Table::RowId Table::addRow(RecordWriter &writer)
{
    RowId rowId = mNextRowId;
    void *data = mTree.add(BTree::Key(&rowId, sizeof(rowId)), writer.dataSize());
    writer.write(data);

    mNextRowId++;

    return rowId;
}

void Table::removeRow(RowId rowId)
{
    mTree.remove(BTree::Key(&rowId, sizeof(rowId)));  
}

void Table::print()
{
    auto printCell = [&](void *data) {
        RecordReader reader(mSchema, data);
        reader.print();
    };

    mTree.print(printCell);
}