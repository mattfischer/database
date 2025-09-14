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
    LeafPage leafPage = findLeaf(rowId);
    return leafPage.lookup(rowId);
}

void *BTree::add(RowId rowId, CellPage::Size size)
{
    LeafPage leafPage = findLeaf(rowId);
    if(leafPage.canAdd(size)) {
        return leafPage.add(rowId, size);
    }
    
    LeafPage newLeafPage = leafPage.split(mPageSet);

    void *ret;
    if(newLeafPage.lowestRowId() < rowId) {
        ret = newLeafPage.add(rowId, size);
    } else {
        ret = leafPage.add(rowId, size);
    }

    Page::Index parentPageIndex = leafPage.parent();
    Page::Index leftSplitIndex = leafPage.page().index();
    RowId leftSplitLowestRowId = leafPage.lowestRowId();
    Page::Index rightSplitIndex = newLeafPage.page().index();
    RowId rightSplitLowestRowId = newLeafPage.lowestRowId();

    while(true) {
        if(parentPageIndex == Page::kInvalidIndex) {
            IndirectPage indirectPage(mPageSet.addPage());
            indirectPage.initialize();

            indirectPage.add(leftSplitLowestRowId, leftSplitIndex);
            indirectPage.add(rightSplitLowestRowId, rightSplitIndex);

            setPageParent(leftSplitIndex, indirectPage.page().index());
            setPageParent(rightSplitIndex, indirectPage.page().index());

            mRootIndex = indirectPage.page().index();
            break;
        } else {
            IndirectPage indirectPage(mPageSet.page(parentPageIndex));
            if(indirectPage.canAdd()) {
                indirectPage.add(rightSplitLowestRowId, rightSplitIndex);
                setPageParent(rightSplitIndex, parentPageIndex);
                break;
            } else {
                IndirectPage newIndirectPage = indirectPage.split(mPageSet);
                for(CellPage::Index index = 0; index < newIndirectPage.cellPage().numCells(); index++) {
                    Page::Index pageIndex = newIndirectPage.cellPageIndex(index);
                    setPageParent(pageIndex, newIndirectPage.page().index());
                }

                if(newIndirectPage.lowestRowId() < rightSplitLowestRowId) {
                    newIndirectPage.add(rightSplitLowestRowId, rightSplitIndex);
                    setPageParent(rightSplitIndex, newIndirectPage.page().index());
                } else {
                    indirectPage.add(rightSplitLowestRowId, rightSplitIndex);
                    setPageParent(rightSplitIndex, indirectPage.page().index());
                }

                parentPageIndex = indirectPage.parent();
                leftSplitIndex = indirectPage.page().index();
                leftSplitLowestRowId = indirectPage.lowestRowId();
                rightSplitIndex = newIndirectPage.page().index();
                rightSplitLowestRowId = newIndirectPage.lowestRowId();
            }
        }
    }

    return ret;
}

void BTree::remove(RowId rowId)
{
    LeafPage leafPage = findLeaf(rowId);
    leafPage.remove(rowId);
}

void BTree::print()
{
    LeafPage leafPage(mPageSet.page(mRootIndex));
    leafPage.print();
}

BTree::LeafPage BTree::findLeaf(RowId rowId)
{
    Page::Index index = mRootIndex;
    while(true) {
        CellPage cellPage(mPageSet.page(index), 0);
        PageHeader *pageHeader = reinterpret_cast<PageHeader*>(cellPage.extraHeader());
        if(pageHeader->type == PageHeader::Leaf) {
            break;
        }

        IndirectPage indirectPage(mPageSet.page(index));
        index = indirectPage.lookup(rowId);
    }

    return LeafPage(mPageSet.page(index));
}

void BTree::setPageParent(Page::Index pageIndex, Page::Index parentIndex)
{
    CellPage cellPage(mPageSet.page(pageIndex), 0);
    PageHeader *pageHeader = reinterpret_cast<PageHeader*>(cellPage.extraHeader());

    pageHeader->parent = parentIndex;
}

BTree::LeafPage::LeafPage(Page &page)
: mCellPage(page, sizeof(PageHeader), 0)
{
}

void BTree::LeafPage::initialize()
{
    mCellPage.initialize();
    header().type = PageHeader::Type::Leaf;
    header().parent = Page::kInvalidIndex;
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

Page::Index BTree::LeafPage::parent()
{
    return header().parent;
}

void BTree::LeafPage::setParent(Page::Index parent)
{
    header().parent = parent;
}

BTree::RowId BTree::LeafPage::lowestRowId()
{
    return cellRowId(0);
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

bool BTree::LeafPage::canAdd(size_t size)
{
    return cellPage().canAllocateCell(size);
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

BTree::LeafPage BTree::LeafPage::split(PageSet &pageSet)
{
    LeafPage newLeafPage(pageSet.addPage());
    newLeafPage.initialize();
    newLeafPage.setParent(parent());

    CellPage::Index begin = cellPage().numCells() / 2;
    CellPage::Index end = cellPage().numCells();
    newLeafPage.cellPage().copyCells(cellPage(), begin, end);

    cellPage().removeCells(begin, end);

    return newLeafPage;
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

BTree::IndirectPage::IndirectPage(Page &page)
: mCellPage(page, sizeof(PageHeader), sizeof(IndirectCell))
{
}

void BTree::IndirectPage::initialize()
{
    mCellPage.initialize();
    header().type = PageHeader::Type::Indirect;
    header().parent = Page::kInvalidIndex;
}

CellPage &BTree::IndirectPage::cellPage()
{
    return mCellPage;
}

Page &BTree::IndirectPage::page()
{
    return mCellPage.page();
}

Page::Index BTree::IndirectPage::parent()
{
    return header().parent;
}

void BTree::IndirectPage::setParent(Page::Index parent)
{
    header().parent = parent;
}

BTree::RowId BTree::IndirectPage::lowestRowId()
{
    return cellRowId(0);
}

BTree::PageHeader &BTree::IndirectPage::header()
{
    return *reinterpret_cast<PageHeader*>(mCellPage.extraHeader());
}

bool BTree::IndirectPage::canAdd()
{
    return cellPage().canAllocateCell(sizeof(IndirectCell));
}

void BTree::IndirectPage::add(RowId rowId, Page::Index pageIndex)
{
    CellPage::Index index = search(rowId);
    if(index < cellPage().numCells() && cellRowId(index) == rowId) {
        return;
    } else {
        cellPage().insertCell(sizeof(IndirectCell), index);
        IndirectCell *indirectCell = reinterpret_cast<IndirectCell*>(cellPage().cell(index));
        indirectCell->rowId = rowId;
        indirectCell->pageIndex = pageIndex;
    }
}

Page::Index BTree::IndirectPage::lookup(RowId rowId)
{
    CellPage::Index index = search(rowId);
    IndirectCell *indirectCell = reinterpret_cast<IndirectCell*>(cellPage().cell(index));
    return indirectCell->pageIndex;
}

BTree::IndirectPage BTree::IndirectPage::split(PageSet &pageSet)
{
    IndirectPage newIndirectPage(pageSet.addPage());
    newIndirectPage.initialize();
    newIndirectPage.setParent(parent());

    CellPage::Index begin = cellPage().numCells() / 2;
    CellPage::Index end = cellPage().numCells();
    newIndirectPage.cellPage().copyCells(cellPage(), begin, end);

    cellPage().removeCells(begin, end);

    return newIndirectPage;
}

CellPage::Index BTree::IndirectPage::search(RowId rowId)
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

Page::Index BTree::IndirectPage::cellPageIndex(CellPage::Index index)
{
    return reinterpret_cast<IndirectCell*>(mCellPage.cell(index))->pageIndex;
}

BTree::RowId BTree::IndirectPage::cellRowId(CellPage::Index index)
{
    return reinterpret_cast<IndirectCell*>(mCellPage.cell(index))->rowId;
}
