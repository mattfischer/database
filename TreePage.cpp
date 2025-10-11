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

TreePage::Key TreePage::cellKey(Index index)
{
    if(index >= header().numCells) {
        return Key();
    }

    if(type() == Type::Indirect && index == 0) {
        return Key();
    }

    Key key;
    key.data = mPage.data(cellKeyOffset(index));

    TreePage::Size keySize = mKeyDefinition.fixedSize();
    if(keySize == 0) {
        keySize = *reinterpret_cast<uint16_t*>(mPage.data(cellOffset(index)));
    }

    key.size = keySize;
    return key;
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

void TreePage::setCellKey(Index index, Key key)
{
    if(key.size == cellKey(index).size) {
        std::memcpy(cellKey(index).data, key.data, key.size);
    } else {
        Header &head = header();
        head.freeSpace += cellSize(index);
        Size dataSize = cellDataSize(index);
        void *src = cellData(index);
        std::vector<uint8_t> data(dataSize);
        std::memcpy(data.data(), src, dataSize);

        uint16_t offset = allocateCell(key, dataSize);
        uint16_t *offsets = offsetsArray();
        offsets[index] = offset;
        
        Size totalKeySize = 0;
        if(key) {
            totalKeySize = mKeyDefinition.fixedSize();
            if(totalKeySize == 0) {
                totalKeySize = sizeof(uint16_t) + key.size;
            }
        }
        std::memcpy(mPage.data(offset + totalKeySize), data.data(), dataSize);

        head.freeSpace -= cellSize(index);
    }
}

void *TreePage::insertCell(Key key, TreePage::Size dataSize, Index index)
{
    Header &head = header();
    uint16_t *offsets = offsetsArray();

    uint16_t offset = allocateCell(key, dataSize);
    std::memmove(offsets + index + 1, offsets + index, (head.numCells - index) * sizeof(uint16_t));
    offsets[index] = offset;
    head.numCells++;

    head.freeSpace -= cellSize(index) + sizeof(uint16_t);

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

TreePage TreePage::split(Index index)
{
    TreePage newPage(pageSet().addPage(), mKeyDefinition);
    newPage.initialize(type());
    newPage.setParent(parent());

    Index begin = index;
    Index end = numCells();

    for(Index i=begin; i<end; i++) {
        if(newPage.type() == Type::Indirect) {
            TreePage movedChild = getPage(indirectPageIndex(i));
            newPage.indirectPushTail(cellKey(i), movedChild);
        } else {
            Size dataSize = cellDataSize(i);
            Index index = newPage.numCells();
            newPage.insertCell(cellKey(i), dataSize, index);

            void *src = cellData(i);
            void *dst = newPage.cellData(index);
            std::memcpy(dst, src, dataSize);
        }
    }

    removeCells(begin, end);

    return newPage;
}

bool TreePage::isDeficient()
{
    return freeSpace() > page().size() / 2;
}

bool TreePage::canSupplyItem(Index index)
{
    return freeSpace() + cellSize(index) + sizeof(uint16_t) < page().size() / 2;
}

void *TreePage::leafLookup(Key key)
{
    Index index = search(key);
    if(index < numCells() && keyCompare(cellKey(index), key) == 0) {
        return cellData(index);
    } else {
        return nullptr;
    }
}

bool TreePage::leafCanAdd(size_t keySize, size_t dataSize)
{
    return canAllocateCell(keySize, dataSize);
}

void *TreePage::leafAdd(Key key, size_t dataSize)
{
    Index index = search(key);
    if(index < numCells() && keyCompare(cellKey(index), key) == 0) {
        return nullptr;
    } else {
        return insertCell(key, dataSize, index);
    }
}

void TreePage::leafRemove(Key key)
{
    Index index = search(key);
    if(index < numCells() && keyCompare(cellKey(index), key) == 0) {
        removeCell(index);
    }
}
bool TreePage::indirectCanAdd(size_t keySize)
{
    return canAllocateCell(keySize, sizeof(Page::Index));
}

void TreePage::indirectAdd(Key key, TreePage &childPage)
{
    if(!key) {
        Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::insertCell(key, sizeof(Page::Index), 0));
        *indexData = childPage.pageIndex();
        childPage.setParent(pageIndex());
    } else {
        Index index = search(key);
        if(index < numCells() && keyCompare(cellKey(index), key) == 0) {
            return;
        } else {
            Page::Index *indexData = reinterpret_cast<Page::Index*>(TreePage::insertCell(key, sizeof(Page::Index), index));
            *indexData = childPage.pageIndex();
            childPage.setParent(pageIndex());
        }
    }
}

Page::Index TreePage::indirectLookup(Key key)
{
    Index index = search(key);
    if(index == numCells() || keyCompare(cellKey(index), key) > 0) {
        index -= 1;
    }

    return indirectPageIndex(index);
}

Page::Index TreePage::indirectPageIndex(Index index)
{
    Page::Index *indexData = reinterpret_cast<Page::Index*>(cellData(index));
    return *indexData;
}

TreePage::KeyValue TreePage::indirectRectifyDeficientChild(TreePage &childPage, Key removedKey)
{
    Index childIndex = search(removedKey);

    if(childIndex == numCells() || keyCompare(cellKey(childIndex), removedKey) > 0) {
        childIndex -= 1;
    }

    if(childIndex < numCells() - 1) {
        TreePage rightNeighbor = getPage(indirectPageIndex(childIndex + 1));
        if(rightNeighbor.canSupplyItem(0)) {
            indirectRotateLeft(childPage, rightNeighbor, childIndex);
            return KeyValue();
        } else {
            KeyValue removedKey = cellKey(childIndex + 1);
            indirectMergeChildren(childPage, rightNeighbor, childIndex + 1);
            return removedKey;
        }
    } else {
        TreePage leftNeighbor = getPage(indirectPageIndex(childIndex - 1));
        if(leftNeighbor.canSupplyItem(leftNeighbor.numCells() - 1)) {
            indirectRotateRight(leftNeighbor, childPage, childIndex);
            return KeyValue();
        } else {
            KeyValue removedKey = cellKey(childIndex);
            indirectMergeChildren(leftNeighbor, childPage, childIndex);
            return removedKey;
        }
    }
}

void TreePage::indirectRotateRight(TreePage &leftChild, TreePage &rightChild, Index index)
{
    if(rightChild.type() == TreePage::Type::Indirect) {
        TreePage movedChild = getPage(leftChild.indirectPageIndex(leftChild.numCells() - 1));
        rightChild.indirectPushHead(cellKey(index), movedChild);
        setCellKey(index, leftChild.cellKey(leftChild.numCells() - 1));
        leftChild.indirectPopTail();
    } else {
        Size size = leftChild.cellDataSize(leftChild.numCells() - 1);
        void *dst = rightChild.insertCell(leftChild.cellKey(leftChild.numCells() - 1), size, 0);
        void *src = leftChild.cellData(leftChild.numCells() - 1);
        std::memcpy(dst, src, size);
        setCellKey(index, leftChild.cellKey(leftChild.numCells() - 1));
        leftChild.removeCell(leftChild.numCells() - 1);
    }
}

void TreePage::indirectRotateLeft(TreePage &leftChild, TreePage &rightChild, Index index)
{
    if(rightChild.type() == TreePage::Type::Indirect) {
        TreePage movedChild = getPage(rightChild.indirectPageIndex(0));
        leftChild.indirectPushTail(cellKey(index + 1), movedChild);
        setCellKey(index + 1, rightChild.cellKey(1));
        rightChild.indirectPopHead();
    } else {
        Size dataSize = rightChild.cellDataSize(0);
        void *dst = leftChild.insertCell(rightChild.cellKey(0), dataSize, leftChild.numCells());
        void *src = rightChild.cellData(0);
        std::memcpy(dst, src, dataSize);
        setCellKey(index + 1, rightChild.cellKey(1));
        rightChild.removeCell(0);
    }
}

void TreePage::indirectMergeChildren(TreePage &leftChild, TreePage &rightChild, Index index)
{
    for(Index i=0; i<rightChild.numCells(); i++) {
        if(leftChild.type() == TreePage::Type::Indirect) {
            TreePage movedChild = getPage(rightChild.indirectPageIndex(i));
            if(i == 0) {
                leftChild.indirectPushTail(cellKey(index), movedChild);
            } else {
                leftChild.indirectPushTail(rightChild.cellKey(i), movedChild);
            }
        } else {
            Size dataSize = rightChild.cellDataSize(i);
            void *dst = leftChild.insertCell(rightChild.cellKey(i), dataSize, leftChild.numCells());
            void *src = rightChild.cellData(i);
            std::memcpy(dst, src, dataSize);
        }
    }
    removeCell(index);
    pageSet().deletePage(rightChild.page());
}

void TreePage::indirectPushHead(Key oldHeadKey, TreePage &childPage)
{
    setCellKey(0, oldHeadKey);
    void *dst = insertCell(Key(), sizeof(Page::Index), 0);
    *reinterpret_cast<Page::Index*>(dst) = childPage.pageIndex();
    childPage.setParent(pageIndex());
}

void TreePage::indirectPushTail(Key key, TreePage &childPage)
{
    if(numCells() == 0) {
        key = Key();
    }

    void *dst = TreePage::insertCell(key, sizeof(Page::Index), numCells());
    *reinterpret_cast<Page::Index*>(dst) = childPage.pageIndex();
    childPage.setParent(pageIndex());  
}
    
void TreePage::indirectPopHead()
{
    setCellKey(1, Key());
    removeCell(0);
}

void TreePage::indirectPopTail()
{
    removeCell(numCells() - 1);
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

uint16_t TreePage::allocateCell(Key key, Size dataSize)
{
    Header &head = header();
    uint16_t arrayEnd = sizeof(Header) + (head.numCells + 1) * sizeof(uint16_t);

    Size totalKeySize = 0;
    if(key) {
        totalKeySize = mKeyDefinition.fixedSize();
        if(totalKeySize == 0) {
            totalKeySize = key.size + sizeof(uint16_t);
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
            *reinterpret_cast<uint16_t*>(mPage.data(offset)) = key.size;
            std::memcpy(mPage.data(offset + sizeof(uint16_t)), key.data, key.size);
        } else {
            std::memcpy(mPage.data(offset), key.data, mKeyDefinition.fixedSize());
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

TreePage::Index TreePage::search(Key key)
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
                Key startKey = cellKey(start);
                int cmp = keyCompare(key, startKey);
                if(cmp <= 0) {
                    return start;
                } else {
                    return end;
                }
            }
        }

        Index mid = (start + end) / 2;
        Key midKey = cellKey(mid);
        int cmp = keyCompare(key, midKey);
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

int TreePage::keyCompare(Key a, Key b)
{
    return mKeyDefinition.compare(a, b);
}

TreePage TreePage::getPage(Page::Index index)
{
    return TreePage(pageSet().page(index), mKeyDefinition);
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