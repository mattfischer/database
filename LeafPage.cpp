#include "LeafPage.hpp"

#include "IndirectPage.hpp"

#include <iostream>

LeafPage::LeafPage(Page &page)
: TreePage(page, 0)
{
}

void LeafPage::initialize()
{
    TreePage::initialize(TreePage::Type::Leaf);
}

void *LeafPage::lookup(RowId rowId)
{
    CellPage::Index index = search(rowId);
    if(index < CellPage::numCells() && cellRowId(index) == rowId) {
        return cellData(index);
    } else {
        return nullptr;
    }
}

bool LeafPage::canAdd(size_t size)
{
    return TreePage::canAllocateCell(size);
}

void *LeafPage::add(RowId rowId, size_t size)
{
    CellPage::Index index = search(rowId);
    if(index < CellPage::numCells() && cellRowId(index) == rowId) {
        return nullptr;
    } else {
        if(index == 0 && parent() != Page::kInvalidIndex) {
            IndirectPage indirectPage(page().pageSet().page(parent()));
            indirectPage.updateRowId(lowestRowId(), rowId);
        }
        return TreePage::insertCell(rowId, size, index);
    }
}

void LeafPage::remove(RowId rowId)
{
    CellPage::Index index = search(rowId);
    if(index < CellPage::numCells() && cellRowId(index) == rowId) {
        CellPage::removeCell(index);
    }
}

LeafPage LeafPage::split()
{
    TreePage newPage = TreePage::split();
    return LeafPage(newPage.page());
}

void LeafPage::print()
{
    for(CellPage::Index i=0; i<CellPage::numCells(); i++) {
        std::cout << std::hex << std::showbase << cellRowId(i) << ": " << CellPage::cellSize(i) << std::endl;
    }
}
