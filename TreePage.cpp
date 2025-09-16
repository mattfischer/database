#include "TreePage.hpp"

TreePage::TreePage(Page &page, Size fixedCellSize)
: CellPage(page, sizeof(Header), fixedCellSize ? fixedCellSize + sizeof(RowId) : 0)
{
}

void TreePage::initialize(Type type)
{
    CellPage::initialize();
    header().type = type;
    header().parent = Page::kInvalidIndex;
}

TreePage::Type TreePage::type()
{
    return header().type;
}

Page::Index TreePage::parent()
{
    return header().parent;
}

void TreePage::setParent(Page::Index parent)
{
    header().parent = parent;
}

TreePage::RowId TreePage::lowestRowId()
{
    return cellRowId(0);
}

TreePage::Header &TreePage::header()
{
    return *reinterpret_cast<Header*>(CellPage::extraHeader());
}

CellPage::Index TreePage::search(RowId rowId)
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

TreePage::RowId TreePage::cellRowId(CellPage::Index index)
{
    return *reinterpret_cast<RowId*>(CellPage::cell(index));
}

void *TreePage::cellData(CellPage::Index index)
{
    return reinterpret_cast<uint8_t*>(cell(index)) + sizeof(RowId);
}

void *TreePage::insertCell(RowId rowId, CellPage::Size size, CellPage::Index index)
{
    CellPage::insertCell(size + sizeof(TreePage::RowId), index);
    void *cellData = CellPage::cell(index);
    *reinterpret_cast<TreePage::RowId*>(cellData) = rowId;

    if(index == 0) {
        Page::Index parentIndex = parent();
    }
    return reinterpret_cast<uint8_t*>(cellData) + sizeof(RowId);
}

bool TreePage::canAllocateCell(CellPage::Size size)
{
    return CellPage::canAllocateCell(size + sizeof(RowId));
}

TreePage TreePage::split(PageSet &pageSet)
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

TreePage::Type TreePage::pageType(Page &page)
{
    TreePage treePage(page, 0);
    return treePage.type();
}
