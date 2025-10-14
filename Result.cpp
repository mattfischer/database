#include "Result.hpp"
#include "Table.hpp"

Result::Result(Table &table, BTree::Pointer start)
: mTable(table)
, mPointer(start)
{
}

bool Result::valid()
{
    return data();
}

void Result::prev()
{
    mPointer = mTable.tree().prev(mPointer);
}

void Result::next()
{
    mPointer = mTable.tree().next(mPointer);
}

void *Result::data()
{
    return mTable.tree().data(mPointer);
}