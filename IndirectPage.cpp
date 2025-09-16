#include "IndirectPage.hpp"

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
    auto [newPage, splitRow] = TreePage::split();
    IndirectPage newIndirectPage(newPage.page());
    newIndirectPage.setCellRowId(0, TreePage::kInvalidRowId);

    for(CellPage::Index index = 0; index < newIndirectPage.CellPage::numCells(); index++) {
        Page::Index childPageIndex = newIndirectPage.cellPageIndex(index);
        Page &childPage = page().pageSet().page(childPageIndex);
        TreePage(childPage).setParent(newIndirectPage.page().index());
    }

    return {newIndirectPage, splitRow};
}

Page::Index IndirectPage::cellPageIndex(CellPage::Index index)
{
    Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::cellData(index));
    return *indexData;
}
