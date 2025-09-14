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
    Page::Index rightSplitIndex = newLeafPage.page().index();

    while(true) {
        TreePage leftSplitPage(mPageSet.page(leftSplitIndex));
        TreePage rightSplitPage(mPageSet.page(rightSplitIndex));

        if(parentPageIndex == Page::kInvalidIndex) {
            IndirectPage indirectPage(mPageSet.addPage());
            indirectPage.initialize();

            indirectPage.add(leftSplitPage);
            indirectPage.add(rightSplitPage);

            mRootIndex = indirectPage.page().index();
            break;
        } else {
            IndirectPage indirectPage(mPageSet.page(parentPageIndex));
            if(indirectPage.canAdd()) {
                indirectPage.add(rightSplitPage);
                break;
            } else {
                IndirectPage newIndirectPage = indirectPage.split(mPageSet);

                if(newIndirectPage.lowestRowId() < rightSplitPage.lowestRowId()) {
                    newIndirectPage.add(rightSplitPage);
                } else {
                    indirectPage.add(rightSplitPage);
                }

                parentPageIndex = indirectPage.parent();
                leftSplitIndex = indirectPage.page().index();
                rightSplitIndex = newIndirectPage.page().index();
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
        TreePage treePage(mPageSet.page(index), 0);
        if(treePage.type() == TreePage::Type::Leaf) {
            break;
        }

        IndirectPage indirectPage(mPageSet.page(index));
        index = indirectPage.lookup(rowId);
    }

    return LeafPage(mPageSet.page(index));
}

BTree::TreePage::TreePage(Page &page, Size fixedCellSize)
: CellPage(page, sizeof(Header), fixedCellSize ? fixedCellSize + sizeof(RowId) : 0)
{
}

void BTree::TreePage::initialize(Type type)
{
    CellPage::initialize();
    header().type = type;
    header().parent = Page::kInvalidIndex;
}

BTree::TreePage::Type BTree::TreePage::type()
{
    return header().type;
}

Page::Index BTree::TreePage::parent()
{
    return header().parent;
}

void BTree::TreePage::setParent(Page::Index parent)
{
    header().parent = parent;
}

BTree::RowId BTree::TreePage::lowestRowId()
{
    return cellRowId(0);
}

BTree::TreePage::Header &BTree::TreePage::header()
{
    return *reinterpret_cast<Header*>(CellPage::extraHeader());
}

CellPage::Index BTree::TreePage::search(RowId rowId)
{
    CellPage::Index start = 0;
    CellPage::Index end = CellPage::numCells();

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

BTree::RowId BTree::TreePage::cellRowId(CellPage::Index index)
{
    return *reinterpret_cast<RowId*>(CellPage::cell(index));
}

void *BTree::TreePage::cellData(CellPage::Index index)
{
    return reinterpret_cast<uint8_t*>(cell(index)) + sizeof(RowId);
}

void *BTree::TreePage::insertCell(RowId rowId, CellPage::Size size, CellPage::Index index)
{
    CellPage::insertCell(size + sizeof(BTree::RowId), index);
    void *cellData = CellPage::cell(index);
    *reinterpret_cast<BTree::RowId*>(cellData) = rowId;
    return reinterpret_cast<uint8_t*>(cellData) + sizeof(RowId);
}

bool BTree::TreePage::canAllocateCell(CellPage::Size size)
{
    return CellPage::canAllocateCell(size + sizeof(RowId));
}

BTree::TreePage BTree::TreePage::split(PageSet &pageSet)
{
    TreePage newPage(pageSet.addPage());
    newPage.initialize(header().type);
    newPage.setParent(parent());

    CellPage::Index begin = CellPage::numCells() / 2;
    CellPage::Index end = CellPage::numCells();
    newPage.CellPage::copyCells(*this, begin, end);

    CellPage::removeCells(begin, end);

    return newPage;
}

BTree::LeafPage::LeafPage(Page &page)
: TreePage(page, 0)
{
}

void BTree::LeafPage::initialize()
{
    TreePage::initialize(TreePage::Type::Leaf);
}

void *BTree::LeafPage::lookup(RowId rowId)
{
    CellPage::Index index = search(rowId);
    if(index < CellPage::numCells() && cellRowId(index) == rowId) {
        return cellData(index);
    } else {
        return nullptr;
    }
}

bool BTree::LeafPage::canAdd(size_t size)
{
    return TreePage::canAllocateCell(size);
}

void *BTree::LeafPage::add(RowId rowId, size_t size)
{
    CellPage::Index index = search(rowId);
    if(index < CellPage::numCells() && cellRowId(index) == rowId) {
        return nullptr;
    } else {
        return TreePage::insertCell(rowId, size, index);
    }
}

void BTree::LeafPage::remove(RowId rowId)
{
    CellPage::Index index = search(rowId);
    if(index < CellPage::numCells() && cellRowId(index) == rowId) {
        CellPage::removeCell(index);
    }
}

BTree::LeafPage BTree::LeafPage::split(PageSet &pageSet)
{
    TreePage newPage = TreePage::split(pageSet);
    return LeafPage(newPage.page());
}

void BTree::LeafPage::print()
{
    for(CellPage::Index i=0; i<CellPage::numCells(); i++) {
        std::cout << std::hex << std::showbase << cellRowId(i) << ": " << CellPage::cellSize(i) << std::endl;
    }
}

BTree::IndirectPage::IndirectPage(Page &page)
: TreePage(page, sizeof(Page::Index))
{
}

void BTree::IndirectPage::initialize()
{
    TreePage::initialize(TreePage::Type::Indirect);
}

bool BTree::IndirectPage::canAdd()
{
    return TreePage::canAllocateCell(sizeof(Page::Index));
}

void BTree::IndirectPage::add(TreePage &page)
{
    CellPage::Index index = search(page.lowestRowId());
    if(index < CellPage::numCells() && cellRowId(index) == page.lowestRowId()) {
        return;
    } else {
        Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::insertCell(page.lowestRowId(), sizeof(Page::Index), index));
        *indexData = page.page().index();
        page.setParent(TreePage::page().index());
    }
}

Page::Index BTree::IndirectPage::lookup(RowId rowId)
{
    CellPage::Index index = search(rowId);
    return cellPageIndex(index);
}

BTree::IndirectPage BTree::IndirectPage::split(PageSet &pageSet)
{
    TreePage newPage = TreePage::split(pageSet);
    IndirectPage newIndirectPage(newPage.page());

    for(CellPage::Index index = 0; index < newIndirectPage.CellPage::numCells(); index++) {
        Page::Index pageIndex = newIndirectPage.cellPageIndex(index);
        TreePage(pageSet.page(pageIndex)).setParent(newIndirectPage.page().index());
    }

    return newIndirectPage;
}

Page::Index BTree::IndirectPage::cellPageIndex(CellPage::Index index)
{
    Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::cellData(index));
    return *indexData;
}
