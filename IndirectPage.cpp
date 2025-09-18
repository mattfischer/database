#include "IndirectPage.hpp"

#include <iostream>

IndirectPage::IndirectPage(Page &page)
: TreePage(page, sizeof(Page::Index))
{
}

void IndirectPage::initialize()
{
    TreePage::initialize(TreePage::Type::Indirect);
}

bool IndirectPage::canAdd()
{
    return TreePage::canAllocateCell(sizeof(Page::Index));
}

void IndirectPage::add(RowId rowId, TreePage &childPage)
{
    CellPage::Index index = search(rowId);
    if(index < CellPage::numCells() && cellRowId(index) == rowId) {
        return;
    } else {
        Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::insertCell(rowId, sizeof(Page::Index), index));
        *indexData = childPage.page().index();
        childPage.setParent(TreePage::page().index());
    }
}

Page::Index IndirectPage::lookup(RowId rowId)
{
    CellPage::Index index = search(rowId);
    if(index == numCells() || cellRowId(index) > rowId) {
        index -= 1;
    }

    return cellPageIndex(index);
}

std::tuple<IndirectPage, TreePage::RowId> IndirectPage::split()
{
    IndirectPage newPage(page().pageSet().addPage());
    newPage.initialize();

    RowId splitRow = TreePage::split(newPage);
    newPage.setCellRowId(0, TreePage::kInvalidRowId);

    for(CellPage::Index index = 0; index < newPage.CellPage::numCells(); index++) {
        Page::Index childPageIndex = newPage.cellPageIndex(index);
        Page &childPage = page().pageSet().page(childPageIndex);
        TreePage(childPage).setParent(newPage.page().index());
    }

    return {newPage, splitRow};
}

TreePage::RowId IndirectPage::rectifyDeficientChild(TreePage &childPage, RowId removedRowId)
{
    CellPage::Index childIndex = search(removedRowId);
    if(childIndex == numCells() || cellRowId(childIndex) > removedRowId) {
        childIndex -= 1;
    }

    if(childIndex < numCells() - 1) {
        TreePage rightNeighbor(page().pageSet().page(cellPageIndex(childIndex + 1)), (childPage.fixedCellSize() == 0) ? 0 : childPage.fixedCellSize() - sizeof(RowId));
        if(rightNeighbor.canSupplyItem()) {
            // shift item from right neighbor
            CellPage::Size size = rightNeighbor.cellDataSize(0);
            void *dst = childPage.insertCell(cellRowId(childIndex + 1), size, childPage.numCells());
            void *src = rightNeighbor.cellData(0);
            std::memcpy(dst, src, size);
            setCellRowId(childIndex + 1, rightNeighbor.cellRowId(1));
            rightNeighbor.removeCell(0);
            if(rightNeighbor.type() == TreePage::Type::Indirect) {
                rightNeighbor.setCellRowId(0, kInvalidRowId);
                IndirectPage indirectChildPage(childPage.page());
                TreePage movedPage(page().pageSet().page(indirectChildPage.cellPageIndex(indirectChildPage.numCells() - 1)));
                movedPage.setParent(childPage.page().index());
            }
            return TreePage::kInvalidRowId;
        } else {
            // merge with right neighbor
            for(CellPage::Index i=0; i<rightNeighbor.numCells(); i++) {
                CellPage::Size size = rightNeighbor.cellDataSize(i);
                RowId rowId = (rightNeighbor.type() == TreePage::Type::Indirect && i == 0) ? cellRowId(childIndex + 1) : rightNeighbor.cellRowId(i);
                void *dst = childPage.insertCell(rowId, size, childPage.numCells());
                void *src = rightNeighbor.cellData(i);
                std::memcpy(dst, src, size);
                if(childPage.type() == TreePage::Type::Indirect) {
                    IndirectPage indirectChildPage(childPage.page());
                    TreePage movedPage(page().pageSet().page(indirectChildPage.cellPageIndex(childPage.numCells() - 1)));
                    movedPage.setParent(childPage.page().index());
                }
            }
            removedRowId = cellRowId(childIndex + 1);
            removeCell(childIndex + 1);
            page().pageSet().deletePage(rightNeighbor.page());
            
            return removedRowId;
        }
    } else {
        TreePage leftNeighbor(page().pageSet().page(cellPageIndex(childIndex - 1)), (childPage.fixedCellSize() == 0) ? 0 : childPage.fixedCellSize() - sizeof(RowId));
        if(leftNeighbor.canSupplyItem()) {
            // shift item from left neighbor
            childPage.setCellRowId(0, cellRowId(childIndex));
            CellPage::Size size = leftNeighbor.cellDataSize(leftNeighbor.numCells() - 1);
            void *dst = childPage.insertCell(leftNeighbor.cellRowId(leftNeighbor.numCells() - 1), size, 0);
            void *src = leftNeighbor.cellData(leftNeighbor.numCells() - 1);
            std::memcpy(dst, src, size);

            setCellRowId(childIndex, childPage.cellRowId(0));
            if(childPage.type() == TreePage::Type::Indirect) {
                childPage.setCellRowId(0, kInvalidRowId);

                IndirectPage indirectChildPage(childPage.page());
                TreePage movedPage(page().pageSet().page(indirectChildPage.cellPageIndex(0)));
                movedPage.setParent(childPage.page().index());
            }
            leftNeighbor.removeCell(leftNeighbor.numCells() - 1);
            return TreePage::kInvalidRowId;
        } else {
            // merge with left neighbor
            for(CellPage::Index i=0; i<childPage.numCells(); i++) {
                CellPage::Size size = childPage.cellDataSize(i);
                RowId rowId = (childPage.type() == TreePage::Type::Indirect && i == 0) ? cellRowId(childIndex) : childPage.cellRowId(i);
                void *dst = leftNeighbor.insertCell(rowId, size, leftNeighbor.numCells());
                void *src = childPage.cellData(i);
                std::memcpy(dst, src, size);
                if(leftNeighbor.type() == TreePage::Type::Indirect) {
                    IndirectPage indirectLeftNeighbor(leftNeighbor.page());
                    TreePage movedPage(page().pageSet().page(indirectLeftNeighbor.cellPageIndex(leftNeighbor.numCells() - 1)));
                    movedPage.setParent(leftNeighbor.page().index());
                }
            }
            removedRowId = cellRowId(childIndex);
            removeCell(childIndex);
            page().pageSet().deletePage(childPage.page());
            return removedRowId;
        }
    }
}

void IndirectPage::print(const std::string &prefix)
{
    std::cout << prefix << "# Indirect page " << page().index();
    if(parent() != Page::kInvalidIndex) {
        std::cout << " (parent " << parent() << ")";
    }
    std::cout << std::endl;

    for(CellPage::Index i=0; i<CellPage::numCells(); i++) {
        std::cout << prefix;
        if(i == 0) {
            std::cout << "_";
        } else {
            std::cout << cellRowId(i);
        }
        std::cout << ": " << std::endl;

        Page::Index index = cellPageIndex(i);
        TreePage childPage(page().pageSet().page(index));
        TreePage::printPage(childPage.page(), prefix + "  ");
    }
}

Page::Index IndirectPage::cellPageIndex(CellPage::Index index)
{
    Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::cellData(index));
    return *indexData;
}
