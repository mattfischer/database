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

void IndirectPage::add(TreePage &childPage)
{
    CellPage::Index index = search(childPage.lowestRowId());
    if(index < CellPage::numCells() && cellRowId(index) == childPage.lowestRowId()) {
        return;
    } else {
        if(index == 0 && parent() != Page::kInvalidIndex) {
            IndirectPage indirectPage(page().pageSet().page(parent()));
            indirectPage.updateRowId(lowestRowId(), childPage.lowestRowId());
        }

        Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::insertCell(childPage.lowestRowId(), sizeof(Page::Index), index));
        *indexData = childPage.page().index();
        childPage.setParent(TreePage::page().index());
    }
}

Page::Index IndirectPage::lookup(RowId rowId)
{
    CellPage::Index index = search(rowId);
    return cellPageIndex(index);
}

IndirectPage IndirectPage::split()
{
    TreePage newPage = TreePage::split();
    IndirectPage newIndirectPage(newPage.page());

    for(CellPage::Index index = 0; index < newIndirectPage.CellPage::numCells(); index++) {
        Page::Index childPageIndex = newIndirectPage.cellPageIndex(index);
        Page &childPage = page().pageSet().page(childPageIndex);
        TreePage(childPage).setParent(newIndirectPage.page().index());
    }

    return newIndirectPage;
}

Page::Index IndirectPage::cellPageIndex(CellPage::Index index)
{
    Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::cellData(index));
    return *indexData;
}
