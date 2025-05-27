#include "BTree.hpp"

#include <iostream>

BTree::BTree(PageSet &pageSet, Page::Index rootIndex)
: mPageSet(pageSet)
{
    mRootIndex = rootIndex;
}

void BTree::intialize()
{
    LeafPage leafPage(mPageSet.page(mRootIndex));
    leafPage.initialize();
}

void *BTree::lookup(RowId rowId)
{
    LeafPage leafPage(mPageSet.page(mRootIndex));
    return leafPage.lookup(rowId);
}

void *BTree::add(RowId rowId, CellPage::Size size)
{
    LeafPage leafPage(mPageSet.page(mRootIndex));
    return leafPage.add(rowId, size);
}

void BTree::remove(RowId rowId)
{
    LeafPage leafPage(mPageSet.page(mRootIndex));
    leafPage.remove(rowId);
}

void BTree::print()
{
    LeafPage leafPage(mPageSet.page(mRootIndex));
    leafPage.print();
}

BTree::LeafPage::LeafPage(Page &page)
: mCellPage(page, sizeof(PageHeader), 0)
{
}

void BTree::LeafPage::initialize()
{
    mCellPage.initialize();
    header().type = PageHeader::Type::Leaf;
}

CellPage &BTree::LeafPage::cellPage()
{
    return mCellPage;
}

Page &BTree::LeafPage::page()
{
    return mCellPage.page();
}

BTree::PageHeader &BTree::LeafPage::header()
{
    return *reinterpret_cast<PageHeader*>(mCellPage.extraHeader());
}

CellPage::Index BTree::LeafPage::search(RowId rowId)
{
    CellPage::Index start = 0;
    CellPage::Index end = mCellPage.numCells();

    while(true) {
        if(end == start) {
            return start;
        }

        if(end == start + 1) {
            RowId startRowId = cellRowId(start);
            if(rowId <= startRowId) {
                return start;
            } else {
                return end;
            }
        }

        CellPage::Index mid = (start + end) / 2;
        RowId midRowId = cellRowId(mid);
        if(rowId == midRowId) {
            return mid;
        }

        if(rowId < midRowId) {
            end = mid;
        } else {
            start = mid;
        }
    }
}

void *BTree::LeafPage::lookup(RowId rowId)
{
    CellPage::Index index = search(rowId);
    if(index < cellPage().numCells() && cellRowId(index) == rowId) {
        void *cellData = cellPage().cell(index);
        return reinterpret_cast<uint8_t*>(cellData) + sizeof(RowId);
    } else {
        return nullptr;
    }
}

void *BTree::LeafPage::add(RowId rowId, size_t size)
{
    CellPage::Index index = search(rowId);
    if(index < cellPage().numCells() && cellRowId(index) == rowId) {
        return nullptr;
    } else {
        cellPage().insertCell(size + sizeof(BTree::RowId), index);
        void *cellData = cellPage().cell(index);
        *reinterpret_cast<BTree::RowId*>(cellData) = rowId;
        return reinterpret_cast<uint8_t*>(cellData) + sizeof(BTree::RowId);
    }
}

void BTree::LeafPage::remove(RowId rowId)
{
    CellPage::Index index = search(rowId);
    if(index < cellPage().numCells() && cellRowId(index) == rowId) {
        cellPage().removeCell(index);
    }
}

BTree::RowId BTree::LeafPage::cellRowId(CellPage::Index index)
{
    return *reinterpret_cast<RowId*>(mCellPage.cell(index));
}

void BTree::LeafPage::print()
{
    for(CellPage::Index i=0; i<mCellPage.numCells(); i++) {
        std::cout << std::hex << std::showbase << cellRowId(i) << ": " << mCellPage.cellSize(i) << std::endl;
    }
}
