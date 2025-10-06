#include "TreePage.hpp"

#include <algorithm>
#include <iostream>

TreePage::TreePage(Page &page, KeyDefinition &keyDefinition)
: mPage(page)
, mKeyDefinition(keyDefinition)
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

Page::Index TreePage::pageIndex()
{
    return mPage.index();
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

void *TreePage::cellKey(Index index)
{
    if(index >= header().numCells) {
        return nullptr;
    }

    return reinterpret_cast<void*>(mPage.data(cellKeyOffset(index)));
}

TreePage::Size TreePage::cellKeySize(Index index)
{
    if(type() == Type::Indirect && index == 0) {
        return 0;
    }

    TreePage::Size keySize = mKeyDefinition.fixedSize();
    if(keySize == 0) {
        keySize = *reinterpret_cast<uint16_t*>(mPage.data(cellOffset(index)));
    }

    return keySize;
}

TreePage::Size TreePage::cellTotalKeySize(Index index)
{
    if(type() == Type::Indirect && index == 0) {
        return 0;
    }

    TreePage::Size keySize = mKeyDefinition.fixedSize();
    if(keySize == 0) {
        keySize = sizeof(uint16_t) + *reinterpret_cast<uint16_t*>(mPage.data(cellOffset(index)));
    }

    return keySize;
}

void *TreePage::cellData(Index index)
{
    if(index >= header().numCells) {
        return nullptr;
    }

    return reinterpret_cast<void*>(mPage.data(cellDataOffset(index)));
}

TreePage::Size TreePage::cellDataSize(Index index)
{
    uint16_t offset = cellOffset(index);
    TreePage::Size keySize = cellTotalKeySize(index);

    switch(type()) {
        case Type::Leaf:
            return *reinterpret_cast<uint16_t*>(mPage.data(offset + keySize));
        
        case Type::Indirect:
            return sizeof(Page::Index);
    }

    return 0;
}

TreePage::Size TreePage::cellTotalDataSize(Index index)
{
    uint16_t offset = cellOffset(index);
    TreePage::Size keySize = cellTotalKeySize(index);

    switch(type()) {
        case Type::Leaf:
            return sizeof(uint16_t) + *reinterpret_cast<uint16_t*>(mPage.data(offset + keySize));
        
        case Type::Indirect:
            return sizeof(Page::Index);
    }

    return 0;
}

TreePage::RowId TreePage::cellRowId(TreePage::Index index)
{
    if(type() == Type::Indirect && index == 0) {
        return kInvalidRowId;
    }

    return *reinterpret_cast<RowId*>(cell(index));
}

void TreePage::setCellRowId(TreePage::Index index, RowId rowId)
{
    *reinterpret_cast<RowId*>(cell(index)) = rowId;
}

void TreePage::setCellKey(Index index, void *key, size_t keySize)
{
    if(keySize == cellKeySize(index)) {
        std::memcpy(cellKey(index), key, keySize);
    } else {
        Header &head = header();
        head.freeSpace += cellSize(index);
        Size dataSize = cellDataSize(index);
        void *src = cellData(index);
        std::vector<uint8_t> data(dataSize);
        std::memcpy(data.data(), src, dataSize);

        uint16_t offset = allocateCell(key, keySize, dataSize);
        uint16_t *offsets = offsetsArray();
        offsets[index] = offset;
        
        Size totalKeySize = 0;
        if(key) {
            totalKeySize = mKeyDefinition.fixedSize();
            if(totalKeySize == 0) {
                totalKeySize = sizeof(uint16_t) + keySize;
            }
        }
        std::memcpy(mPage.data(offset + totalKeySize), data.data(), dataSize);

        head.freeSpace -= cellSize(index);
    }
}

void *TreePage::insertCell(void *key, TreePage::Size keySize, TreePage::Size dataSize, Index index)
{
    Header &head = header();
    uint16_t *offsets = offsetsArray();

    uint16_t offset = allocateCell(key, keySize, dataSize);
    std::memmove(offsets + index + 1, offsets + index, (head.numCells - index) * sizeof(uint16_t));
    offsets[index] = offset;
    head.numCells++;

    head.freeSpace -= cellSize(index) + sizeof(uint16_t);

    return cellData(index);
}

void TreePage::removeCell(Index index)
{
    if(type() == Type::Indirect && index == 0) {
        setCellKey(1, nullptr, 0);
    }

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
        if(type() == Type::Indirect && index == 0) {
            insertCell(nullptr, 0, size, index);
        } else {
            insertCell(page.cellKey(i), page.cellKeySize(i), size, index);
        }

        void *src = page.cellData(i);
        void *dst = cellData(index);
        std::memcpy(dst, src, size);
    }
}

std::tuple<TreePage, TreePage::RowId> TreePage::split()
{
    TreePage newPage(pageSet().addPage(), mKeyDefinition);
    newPage.initialize(type());
    newPage.setParent(parent());

    Index begin = numCells() / 2;
    Index end = numCells();

    RowId splitRow = cellRowId(begin);
    newPage.copyCells(*this, begin, end);

    removeCells(begin, end);

    if(type() == Type::Indirect) {
        for(Index index = 0; index < newPage.numCells(); index++) {
            Page::Index childPageIndex = newPage.indirectPageIndex(index);
            TreePage(pageSet().page(childPageIndex), mKeyDefinition).setParent(newPage.pageIndex());
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

void *TreePage::leafLookup(void *key)
{
    Index index = search(key);
    if(index < numCells() && mKeyDefinition.compare(cellKey(index), key) == 0) {
        return cellData(index);
    } else {
        return nullptr;
    }
}

bool TreePage::leafCanAdd(size_t keySize, size_t dataSize)
{
    return canAllocateCell(keySize, dataSize);
}

void *TreePage::leafAdd(void *key, size_t keySize, size_t dataSize)
{
    Index index = search(key);
    if(index < numCells() && mKeyDefinition.compare(cellKey(index), key) == 0) {
        return nullptr;
    } else {
        return insertCell(key, keySize, dataSize, index);
    }
}

void TreePage::leafRemove(RowId rowId)
{
    Index index = search(&rowId);
    if(index < numCells() && cellRowId(index) == rowId) {
        removeCell(index);
    }
}
bool TreePage::indirectCanAdd()
{
    return canAllocateCell(sizeof(RowId), sizeof(Page::Index));
}

void TreePage::indirectAdd(void *key, size_t keySize, TreePage &childPage)
{
    if(!key) {
        Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::insertCell(key, keySize, sizeof(Page::Index), 0));
        *indexData = childPage.pageIndex();
        childPage.setParent(pageIndex());
    } else {
        Index index = search(key);
        if(index < numCells() && mKeyDefinition.compare(cellKey(index), key) == 0) {
            return;
        } else {
            Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::insertCell(key, keySize, sizeof(Page::Index), index));
            *indexData = childPage.pageIndex();
            childPage.setParent(pageIndex());
        }
    }
}

Page::Index TreePage::indirectLookup(void *key)
{
    Index index = search(key);
    if(index == numCells() || mKeyDefinition.compare(cellKey(index), key) > 0) {
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
    Index childIndex = search(&removedRowId);
    if(childIndex == numCells() || cellRowId(childIndex) > removedRowId) {
        childIndex -= 1;
    }

    if(childIndex < numCells() - 1) {
        TreePage rightNeighbor(pageSet().page(indirectPageIndex(childIndex + 1)), mKeyDefinition);
        if(rightNeighbor.canSupplyItem()) {
            indirectRotateLeft(childPage, rightNeighbor, childIndex);
            return TreePage::kInvalidRowId;
        } else {
            removedRowId = cellRowId(childIndex + 1);
            indirectMergeChildren(childPage, rightNeighbor, childIndex + 1);
            return removedRowId;
        }
    } else {
        TreePage leftNeighbor(pageSet().page(indirectPageIndex(childIndex - 1)), mKeyDefinition);
        if(leftNeighbor.canSupplyItem()) {
            indirectRotateRight(leftNeighbor, childPage, childIndex);
            return TreePage::kInvalidRowId;
        } else {
            removedRowId = cellRowId(childIndex);
            indirectMergeChildren(leftNeighbor, childPage, childIndex);
            return removedRowId;
        }
    }
}

void TreePage::indirectRotateRight(TreePage &leftChild, TreePage &rightChild, Index index)
{
    if(rightChild.type() == TreePage::Type::Indirect) {
        RowId rowId = cellRowId(index);
        TreePage movedChild(pageSet().page(leftChild.indirectPageIndex(leftChild.numCells() - 1)), mKeyDefinition);
        rightChild.indirectPushHead(&rowId, sizeof(RowId), movedChild);
    } else {
        Size size = leftChild.cellDataSize(leftChild.numCells() - 1);
        void *dst = rightChild.insertCell(leftChild.cellKey(leftChild.numCells() - 1), leftChild.cellKeySize(leftChild.numCells() - 1), size, 0);
        void *src = leftChild.cellData(leftChild.numCells() - 1);
        std::memcpy(dst, src, size);
    }

    setCellRowId(index, leftChild.cellRowId(leftChild.numCells() - 1));
    leftChild.removeCell(leftChild.numCells() - 1);
}

void TreePage::indirectRotateLeft(TreePage &leftChild, TreePage &rightChild, Index index)
{
    Size size = rightChild.cellDataSize(0);
    RowId rowId = (leftChild.type() == TreePage::Indirect) ? cellRowId(index + 1) : rightChild.cellRowId(0);
    void *dst = leftChild.insertCell(&rowId, sizeof(RowId), size, leftChild.numCells());
    void *src = rightChild.cellData(0);
    std::memcpy(dst, src, size);
    setCellRowId(index + 1, rightChild.cellRowId(1));
    rightChild.removeCell(0);
    if(rightChild.type() == TreePage::Type::Indirect) {
        TreePage(pageSet().page(leftChild.indirectPageIndex(leftChild.numCells() - 1)), mKeyDefinition).setParent(leftChild.pageIndex());
    }
}

void TreePage::indirectMergeChildren(TreePage &leftChild, TreePage &rightChild, Index index)
{
    for(Index i=0; i<rightChild.numCells(); i++) {
        Size size = rightChild.cellDataSize(i);
        RowId rowId = (rightChild.type() == TreePage::Type::Indirect && i == 0) ? cellRowId(index) : rightChild.cellRowId(i);
        void *dst = leftChild.insertCell(&rowId, sizeof(RowId), size, leftChild.numCells());
        void *src = rightChild.cellData(i);
        std::memcpy(dst, src, size);
        if(leftChild.type() == TreePage::Type::Indirect) {
            TreePage(pageSet().page(leftChild.indirectPageIndex(leftChild.numCells() - 1)), mKeyDefinition).setParent(leftChild.pageIndex());
        }
    }
    removeCell(index);
    pageSet().deletePage(rightChild.page());
}

void TreePage::indirectPushHead(void *oldHeadKey, Size oldHeadKeySize, TreePage &childPage)
{
    setCellKey(0, oldHeadKey, oldHeadKeySize);
    void *dst = insertCell(nullptr, 0, sizeof(Page::Index), 0);
    *reinterpret_cast<Page::Index*>(dst) = childPage.pageIndex();
    childPage.setParent(pageIndex());
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
    return cellTotalKeySize(index) + cellTotalDataSize(index);
}

uint16_t TreePage::cellOffset(Index index)
{ 
    return offsetsArray()[index];
}

uint16_t TreePage::cellKeyOffset(Index index)
{
    uint16_t offset = cellOffset(index);
    if(mKeyDefinition.fixedSize() == 0) {
        offset += sizeof(uint16_t);
    }

    return offset;
}

uint16_t TreePage::cellDataOffset(Index index)
{
    uint16_t offset = cellOffset(index);
    TreePage::Size keySize = cellTotalKeySize(index);

    return offset + keySize + ((type() == Type::Leaf) ? sizeof(uint16_t) : 0);
}

uint16_t TreePage::allocateCell(void *key, Size keySize, Size dataSize)
{
    Header &head = header();
    uint16_t arrayEnd = sizeof(Header) + (head.numCells + 1) * sizeof(uint16_t);

    Size totalKeySize = 0;
    if(key) {
        totalKeySize = mKeyDefinition.fixedSize();
        if(totalKeySize == 0) {
            totalKeySize = keySize + sizeof(uint16_t);
        }
    }

    Size totalDataSize = dataSize;
    if(type() == Type::Leaf) {
        totalDataSize += sizeof(uint16_t);
    }

    Size totalSize = totalKeySize + totalDataSize;
    if(head.dataStart - arrayEnd < totalSize) {
        defragPage();
    }

    uint16_t offset = head.dataStart - totalSize;
    head.dataStart -= totalSize;

    if(key) {
        if(mKeyDefinition.fixedSize() == 0) {
            *reinterpret_cast<uint16_t*>(mPage.data(offset)) = keySize;
            std::memcpy(mPage.data(offset + sizeof(uint16_t)), key, keySize);
        } else {
            std::memcpy(mPage.data(offset), key, mKeyDefinition.fixedSize());
        }
    }

    if(type() == Type::Leaf) {
        *reinterpret_cast<uint16_t*>(mPage.data(offset + totalKeySize)) = dataSize;
    }

    return offset;
}

bool TreePage::canAllocateCell(Size keySize, Size dataSize)
{
    Header &head = header();
    Size totalKeySize = mKeyDefinition.fixedSize();
    if(totalKeySize == 0) {
        totalKeySize = keySize + sizeof(uint16_t);
    }

    Size totalDataSize = dataSize;
    if(type() == Type::Leaf) {
        totalDataSize += sizeof(uint16_t);
    }

    Size totalSize = totalKeySize + totalDataSize;

    return (head.freeSpace >= totalSize + sizeof(uint16_t));
}

TreePage::Index TreePage::search(void *key)
{
    Index start = 0;
    Index end = numCells();

    while(true) {
        if(end == start) {
            return start;
        }

        if(end == start + 1) {
            if(type() == Type::Indirect && start == 0) {
                return end;
            } else {
                void *startKey = cellKey(start);
                int cmp = mKeyDefinition.compare(key, startKey);
                if(cmp <= 0) {
                    return start;
                } else {
                    return end;
                }
            }
        }

        Index mid = (start + end) / 2;
        void *midKey = cellKey(mid);
        int cmp = mKeyDefinition.compare(key, midKey);
        if(cmp == 0) {
            return mid;
        }

        if(cmp < 0) {
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

PageSet &TreePage::pageSet()
{
    return mPage.pageSet();
}