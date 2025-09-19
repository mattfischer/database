#include "TreePage.hpp"

#include <algorithm>
#include <iostream>

TreePage::TreePage(Page &page)
: mPage(page)
{
}

void TreePage::initialize(Type type)
{
    Header &head = header();

    head.type = type;
    head.numCells = 0;
    head.dataStart = mPage.size();
    head.freeSpace = mPage.size() - sizeof(Header);
    head.parent = Page::kInvalidIndex;
}

Page &TreePage::page()
{
    return mPage;
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

TreePage::Size TreePage::freeSpace()
{
    Header &head = header();

    return head.freeSpace;
}

TreePage::Index TreePage::numCells()
{
    return header().numCells;
}

void *TreePage::cellData(TreePage::Index index)
{
    if(index >= header().numCells) {
        return nullptr;
    }

    return reinterpret_cast<void*>(mPage.data(cellDataOffset(index)));
}

TreePage::Size TreePage::cellDataSize(Index index)
{
    switch(type()) {
        case Type::Leaf:
            return *reinterpret_cast<uint16_t*>(mPage.data(cellOffset(index) + sizeof(RowId)));
        
        case Type::Indirect:
            return sizeof(Page::Index);
    }

    return 0;
}

TreePage::RowId TreePage::cellRowId(TreePage::Index index)
{
    return *reinterpret_cast<RowId*>(cell(index));
}

void TreePage::setCellRowId(TreePage::Index index, RowId rowId)
{
    *reinterpret_cast<RowId*>(cell(index)) = rowId;
}

void *TreePage::insertCell(RowId rowId, TreePage::Size size, Index index)
{
    Header &head = header();
    uint16_t *offsets = offsetsArray();

    uint16_t offset = allocateCell(rowId, size);
    std::memmove(offsets + index + 1, offsets + index, (head.numCells - index) * sizeof(uint16_t));
    offsets[index] = offset;
    head.numCells++;

    Size totalSize = sizeof(RowId) + size + ((type() == Type::Leaf) ? sizeof(uint16_t) : 0);
    head.freeSpace -= totalSize + sizeof(uint16_t);

    return cellData(index);
}

void TreePage::removeCell(Index index)
{
    removeCells(index, index + 1);
}

void TreePage::removeCells(Index begin, Index end)
{
    Header &head = header();
    for(Index i = begin; i < end; i++) {
        head.freeSpace += cellSize(i) + sizeof(uint16_t);
    }

    uint16_t *array = offsetsArray();
    std::memmove(array + begin, array + end, (head.numCells - end) * sizeof(uint16_t));
    head.numCells -= (end - begin);
}

void TreePage::copyCells(TreePage &page, Index begin, Index end)
{
    for(Index i=begin; i<end; i++) {
        Size size = page.cellDataSize(i);
        Index index = numCells();
        insertCell(page.cellRowId(i), size, index);

        void *src = page.cell(i);
        void *dst = cell(index);
        std::memcpy(dst, src, size);
    }
}

std::tuple<TreePage, TreePage::RowId> TreePage::split()
{
    TreePage newPage(page().pageSet().addPage());
    newPage.initialize(type());
    newPage.setParent(parent());

    Index begin = numCells() / 2;
    Index end = numCells();

    RowId splitRow = cellRowId(begin);
    newPage.copyCells(*this, begin, end);

    removeCells(begin, end);

    if(type() == Type::Indirect) {
        newPage.setCellRowId(0, kInvalidRowId);

        for(Index index = 0; index < newPage.numCells(); index++) {
            Page::Index childPageIndex = newPage.indirectPageIndex(index);
            Page &childPage = page().pageSet().page(childPageIndex);
            TreePage(childPage).setParent(newPage.page().index());
        }
    }

    return {newPage, splitRow};
}

bool TreePage::isDeficient()
{
    return freeSpace() > page().size() / 2;
}

bool TreePage::canSupplyItem()
{
    return freeSpace() + cellSize(0) < page().size() / 2;
}

void *TreePage::leafLookup(RowId rowId)
{
    Index index = search(rowId);
    if(index < numCells() && cellRowId(index) == rowId) {
        return cellData(index);
    } else {
        return nullptr;
    }
}

bool TreePage::leafCanAdd(size_t size)
{
    return canAllocateCell(size);
}

void *TreePage::leafAdd(RowId rowId, size_t size)
{
    Index index = search(rowId);
    if(index < numCells() && cellRowId(index) == rowId) {
        return nullptr;
    } else {
        return insertCell(rowId, size, index);
    }
}

void TreePage::leafRemove(RowId rowId)
{
    Index index = search(rowId);
    if(index < numCells() && cellRowId(index) == rowId) {
        removeCell(index);
    }
}
bool TreePage::indirectCanAdd()
{
    return canAllocateCell(sizeof(Page::Index));
}

void TreePage::indirectAdd(RowId rowId, TreePage &childPage)
{
    Index index = search(rowId);
    if(index < numCells() && cellRowId(index) == rowId) {
        return;
    } else {
        Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::insertCell(rowId, sizeof(Page::Index), index));
        *indexData = childPage.page().index();
        childPage.setParent(TreePage::page().index());
    }
}

Page::Index TreePage::indirectLookup(RowId rowId)
{
    Index index = search(rowId);
    if(index == numCells() || cellRowId(index) > rowId) {
        index -= 1;
    }

    return indirectPageIndex(index);
}

Page::Index TreePage::indirectPageIndex(Index index)
{
    Page::Index *indexData = reinterpret_cast<Page::Index*>(cellData(index));
    return *indexData;
}

TreePage::RowId TreePage::indirectRectifyDeficientChild(TreePage &childPage, RowId removedRowId)
{
    Index childIndex = search(removedRowId);
    if(childIndex == numCells() || cellRowId(childIndex) > removedRowId) {
        childIndex -= 1;
    }

    if(childIndex < numCells() - 1) {
        TreePage rightNeighbor(page().pageSet().page(indirectPageIndex(childIndex + 1)));
        if(rightNeighbor.canSupplyItem()) {
            // shift item from right neighbor
            Size size = rightNeighbor.cellDataSize(0);
            void *dst = childPage.insertCell(cellRowId(childIndex + 1), size, childPage.numCells());
            void *src = rightNeighbor.cellData(0);
            std::memcpy(dst, src, size);
            setCellRowId(childIndex + 1, rightNeighbor.cellRowId(1));
            rightNeighbor.removeCell(0);
            if(rightNeighbor.type() == TreePage::Type::Indirect) {
                rightNeighbor.setCellRowId(0, kInvalidRowId);
                TreePage movedPage(page().pageSet().page(childPage.indirectPageIndex(childPage.numCells() - 1)));
                movedPage.setParent(childPage.page().index());
            }
            return TreePage::kInvalidRowId;
        } else {
            // merge with right neighbor
            for(Index i=0; i<rightNeighbor.numCells(); i++) {
                Size size = rightNeighbor.cellDataSize(i);
                RowId rowId = (rightNeighbor.type() == TreePage::Type::Indirect && i == 0) ? cellRowId(childIndex + 1) : rightNeighbor.cellRowId(i);
                void *dst = childPage.insertCell(rowId, size, childPage.numCells());
                void *src = rightNeighbor.cellData(i);
                std::memcpy(dst, src, size);
                if(childPage.type() == TreePage::Type::Indirect) {
                    TreePage movedPage(page().pageSet().page(childPage.indirectPageIndex(childPage.numCells() - 1)));
                    movedPage.setParent(childPage.page().index());
                }
            }
            removedRowId = cellRowId(childIndex + 1);
            removeCell(childIndex + 1);
            page().pageSet().deletePage(rightNeighbor.page());
            
            return removedRowId;
        }
    } else {
        TreePage leftNeighbor(page().pageSet().page(indirectPageIndex(childIndex - 1)));
        if(leftNeighbor.canSupplyItem()) {
            // shift item from left neighbor
            childPage.setCellRowId(0, cellRowId(childIndex));
            Size size = leftNeighbor.cellDataSize(leftNeighbor.numCells() - 1);
            void *dst = childPage.insertCell(leftNeighbor.cellRowId(leftNeighbor.numCells() - 1), size, 0);
            void *src = leftNeighbor.cellData(leftNeighbor.numCells() - 1);
            std::memcpy(dst, src, size);

            setCellRowId(childIndex, childPage.cellRowId(0));
            if(childPage.type() == TreePage::Type::Indirect) {
                childPage.setCellRowId(0, kInvalidRowId);

                TreePage movedPage(page().pageSet().page(childPage.indirectPageIndex(0)));
                movedPage.setParent(childPage.page().index());
            }
            leftNeighbor.removeCell(leftNeighbor.numCells() - 1);
            return TreePage::kInvalidRowId;
        } else {
            // merge with left neighbor
            for(Index i=0; i<childPage.numCells(); i++) {
                Size size = childPage.cellDataSize(i);
                RowId rowId = (childPage.type() == TreePage::Type::Indirect && i == 0) ? cellRowId(childIndex) : childPage.cellRowId(i);
                void *dst = leftNeighbor.insertCell(rowId, size, leftNeighbor.numCells());
                void *src = childPage.cellData(i);
                std::memcpy(dst, src, size);
                if(leftNeighbor.type() == TreePage::Type::Indirect) {
                    TreePage movedPage(page().pageSet().page(leftNeighbor.indirectPageIndex(leftNeighbor.numCells() - 1)));
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

void TreePage::print(const std::string &prefix)
{
    switch(type()) {
        case TreePage::Type::Leaf:
            std::cout << prefix << "# Leaf page " << page().index();
            if(parent() != Page::kInvalidIndex) {
                std::cout << " (parent " << parent() << ")";
            }
            std::cout << std::endl;

            for(Index i=0; i<numCells(); i++) {
                std::cout << prefix << cellRowId(i) << ": " << cellDataSize(i) << std::endl;
            }
            break;
        case TreePage::Type::Indirect:
            std::cout << prefix << "# Indirect page " << page().index();
            if(parent() != Page::kInvalidIndex) {
                std::cout << " (parent " << parent() << ")";
            }
            std::cout << std::endl;

            for(Index i=0; i<numCells(); i++) {
                std::cout << prefix;
                if(i == 0) {
                    std::cout << "_";
                } else {
                    std::cout << cellRowId(i);
                }
                std::cout << ": " << std::endl;

                Page::Index index = indirectPageIndex(i);
                TreePage childPage(page().pageSet().page(index));
                childPage.print(prefix + "  ");
            }
            break;
    }
}

TreePage::Header &TreePage::header()
{
    return *reinterpret_cast<Header*>(mPage.data(0));
}

uint16_t *TreePage::offsetsArray()
{
    return reinterpret_cast<uint16_t*>(mPage.data(sizeof(Header)));
}

void *TreePage::cell(Index index)
{
    if(index >= header().numCells) {
        return nullptr;
    }

    return reinterpret_cast<void*>(mPage.data(cellOffset(index)));
}

TreePage::Size TreePage::cellSize(Index index)
{
    switch(type()) {
        case Type::Leaf:
            return sizeof(RowId) + sizeof(uint16_t) + *reinterpret_cast<uint16_t*>(mPage.data(cellOffset(index) + sizeof(RowId)));
        
        case Type::Indirect:
            return sizeof(RowId) + sizeof(Page::Index);
    }

    return 0;
}

uint16_t TreePage::cellOffset(Index index)
{ 
    return offsetsArray()[index];
}

uint16_t TreePage::cellDataOffset(Index index)
{
    uint16_t offset = cellOffset(index);
    return offset + sizeof(RowId) + ((type() == Type::Leaf) ? sizeof(uint16_t) : 0);
}

uint16_t TreePage::allocateCell(RowId rowId, Size size)
{
    Header &head = header();
    uint16_t arrayEnd = sizeof(Header) + (head.numCells + 1) * sizeof(uint16_t);

    Size totalSize = sizeof(RowId) + size + ((type() == Type::Leaf) ? sizeof(uint16_t) : 0);

    if(head.dataStart - arrayEnd < totalSize) {
        defragPage();
    }

    uint16_t offset = head.dataStart - totalSize;
    head.dataStart -= totalSize;
    *reinterpret_cast<RowId*>(mPage.data(offset)) = rowId;
    if(type() == Type::Leaf) {
        *reinterpret_cast<uint16_t*>(mPage.data(offset + sizeof(RowId))) = size;
    }

    return offset;
}

bool TreePage::canAllocateCell(Size size)
{
    Header &head = header();
    Size totalSize = sizeof(RowId) + size + ((type() == Type::Leaf) ? sizeof(uint16_t) : 0);

    return (head.freeSpace >= totalSize + sizeof(uint16_t));
}

TreePage::Index TreePage::search(RowId rowId)
{
    Index start = 0;
    Index end = numCells();

    while(true) {
        if(end == start) {
            return start;
        }

        if(end == start + 1) {
            RowId startRowId = cellRowId(start);
            if(startRowId != kInvalidRowId && rowId <= startRowId) {
                return start;
            } else {
                return end;
            }
        }

        Index mid = (start + end) / 2;
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

void TreePage::defragPage()
{
    Header &head = header();
    uint16_t *offsets = offsetsArray();

    std::vector<Index> indices(head.numCells);
    for(unsigned int i=0; i<head.numCells; i++) {
        indices[i] = i;
    }
    std::sort(indices.begin(), indices.end(), 
        [&](Index a, Index b) { return offsets[a] > offsets[b]; }
    );

    head.dataStart = mPage.size();
    for(Index i : indices) {
        Size size = cellSize(i);
        
        head.dataStart -= size;
        std::memmove(mPage.data(head.dataStart), mPage.data(offsets[i]), size);
        offsets[i] = head.dataStart;
    }
}
