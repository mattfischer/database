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

void IndirectPage::print(const std::string &prefix)
{
    for(CellPage::Index i=0; i<CellPage::numCells(); i++) {
        std::cout << prefix;
        if(i == 0) {
            std::cout << "< " << cellRowId(1);
        } else if(i < CellPage::numCells() - 1) {
            std::cout << cellRowId(i) << "-" << cellRowId(i+1) - 1;
        } else {
            std::cout << ">= " << cellRowId(i);
        }

        Page::Index index = cellPageIndex(i);
        TreePage childPage(page().pageSet().page(index));
        std::cout << " (";
        std::cout << ((childPage.type() == TreePage::Type::Leaf) ? "leaf" : "indirect");
        std::cout << " page " << cellPageIndex(i);
        if(childPage.parent() != Page::kInvalidIndex) {
            std::cout << ", parent " << childPage.parent();
        }
        std::cout << ")" << std::endl;
        TreePage::printPage(childPage.page(), prefix + "  ");
    }
}

Page::Index IndirectPage::cellPageIndex(CellPage::Index index)
{
    Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::cellData(index));
    return *indexData;
}
