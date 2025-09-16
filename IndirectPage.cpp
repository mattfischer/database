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

void IndirectPage::add(TreePage &page)
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

Page::Index IndirectPage::lookup(RowId rowId)
{
    CellPage::Index index = search(rowId);
    return cellPageIndex(index);
}

IndirectPage IndirectPage::split(PageSet &pageSet)
{
    TreePage newPage = TreePage::split(pageSet);
    IndirectPage newIndirectPage(newPage.page());

    for(CellPage::Index index = 0; index < newIndirectPage.CellPage::numCells(); index++) {
        Page::Index pageIndex = newIndirectPage.cellPageIndex(index);
        TreePage(pageSet.page(pageIndex)).setParent(newIndirectPage.page().index());
    }

    return newIndirectPage;
}

Page::Index IndirectPage::cellPageIndex(CellPage::Index index)
{
    Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::cellData(index));
    return *indexData;
}
