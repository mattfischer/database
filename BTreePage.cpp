#include "BTreePage.hpp"

#include <algorithm>
#include <iostream>

BTreePage::BTreePage(Page &page, KeyDefinition &keyDefinition)
: mPage(page)
, mKeyDefinition(keyDefinition)
{
}

void BTreePage::initialize(Type type)
{
    Header &head = header();

    head.type = type;
    head.numCells = 0;
    head.dataStart = mPage.size();
    head.freeSpace = mPage.size() - sizeof(Header);
    head.parent = Page::kInvalidIndex;
    head.prevSibling = Page::kInvalidIndex;
    head.nextSibling = Page::kInvalidIndex;
}

Page &BTreePage::page()
{
    return mPage;
}

Page::Index BTreePage::pageIndex()
{
    return mPage.index();
}

BTreePage::Type BTreePage::type()
{
    return header().type;
}

Page::Index BTreePage::parent()
{
    return header().parent;
}

void BTreePage::setParent(Page::Index parent)
{
    header().parent = parent;
}

Page::Index BTreePage::prevSibling()
{
    return header().prevSibling;
}

void BTreePage::setPrevSibling(Page::Index prevSibling)
{
    header().prevSibling = prevSibling;
}

Page::Index BTreePage::nextSibling()
{
    return header().nextSibling;
}

void BTreePage::setNextSibling(Page::Index nextSibling)
{
    header().nextSibling = nextSibling;
}

BTreePage::Size BTreePage::freeSpace()
{
    Header &head = header();

    return head.freeSpace;
}

BTreePage::Index BTreePage::numCells()
{
    return header().numCells;
}

BTreePage::Key BTreePage::cellKey(Index index)
{
    if(index >= header().numCells) {
        return Key();
    }

    if(type() == Type::Indirect && index == 0) {
        return Key();
    }

    Key key;
    key.data = mPage.data(cellKeyOffset(index));

    BTreePage::Size keySize = mKeyDefinition.fixedSize();
    if(keySize == 0) {
        keySize = *reinterpret_cast<uint16_t*>(mPage.data(cellOffset(index)));
    }

    key.size = keySize;
    return key;
}

BTreePage::Size BTreePage::cellTotalKeySize(Index index)
{
    if(type() == Type::Indirect && index == 0) {
        return 0;
    }

    BTreePage::Size keySize = mKeyDefinition.fixedSize();
    if(keySize == 0) {
        keySize = sizeof(uint16_t) + *reinterpret_cast<uint16_t*>(mPage.data(cellOffset(index)));
    }

    return keySize;
}

void *BTreePage::cellData(Index index)
{
    if(index >= header().numCells) {
        return nullptr;
    }

    return reinterpret_cast<void*>(mPage.data(cellDataOffset(index)));
}

BTreePage::Size BTreePage::cellDataSize(Index index)
{
    uint16_t offset = cellOffset(index);
    BTreePage::Size keySize = cellTotalKeySize(index);

    switch(type()) {
        case Type::Leaf:
            return *reinterpret_cast<uint16_t*>(mPage.data(offset + keySize));
        
        case Type::Indirect:
            return sizeof(Page::Index);
    }

    return 0;
}

BTreePage::Size BTreePage::cellTotalDataSize(Index index)
{
    uint16_t offset = cellOffset(index);
    BTreePage::Size keySize = cellTotalKeySize(index);

    switch(type()) {
        case Type::Leaf:
            return sizeof(uint16_t) + *reinterpret_cast<uint16_t*>(mPage.data(offset + keySize));
        
        case Type::Indirect:
            return sizeof(Page::Index);
    }

    return 0;
}

void BTreePage::setCellKey(Index index, Key key)
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

BTreePage::Index BTreePage::insertCell(Key key, BTreePage::Size dataSize, Index index)
{
    Header &head = header();
    uint16_t *offsets = offsetsArray();

    uint16_t offset = allocateCell(key, dataSize);
    std::memmove(offsets + index + 1, offsets + index, (head.numCells - index) * sizeof(uint16_t));
    offsets[index] = offset;
    head.numCells++;

    head.freeSpace -= cellSize(index) + sizeof(uint16_t);

    return index;
}

void BTreePage::removeCell(Index index)
{
    removeCells(index, index + 1);
}

void BTreePage::removeCells(Index begin, Index end)
{
    Header &head = header();
    for(Index i = begin; i < end; i++) {
        head.freeSpace += cellSize(i) + sizeof(uint16_t);
    }

    uint16_t *array = offsetsArray();
    std::memmove(array + begin, array + end, (head.numCells - end) * sizeof(uint16_t));
    head.numCells -= (end - begin);
}

BTreePage BTreePage::split(Index index)
{
    BTreePage newPage(pageSet().addPage(), mKeyDefinition);
    newPage.initialize(type());
    newPage.setParent(parent());

    newPage.setPrevSibling(pageIndex());
    newPage.setNextSibling(nextSibling());
    if(nextSibling() != Page::kInvalidIndex) {
        getPage(nextSibling()).setPrevSibling(newPage.pageIndex());
    }
    setNextSibling(newPage.pageIndex());

    Index begin = index;
    Index end = numCells();

    for(Index i=begin; i<end; i++) {
        if(newPage.type() == Type::Indirect) {
            BTreePage movedChild = getPage(indirectPageIndex(i));
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

bool BTreePage::isDeficient()
{
    return freeSpace() > page().size() / 2;
}

bool BTreePage::canSupplyItem(Index index)
{
    return freeSpace() + cellSize(index) + sizeof(uint16_t) < page().size() / 2;
}

BTreePage::Index BTreePage::leafLookup(Key key)
{
    Index index = search(key);
    if(index < numCells() && keyCompare(cellKey(index), key) == 0) {
        return index;
    } else {
        return kInvalidIndex;
    }
}

bool BTreePage::leafCanAdd(size_t keySize, size_t dataSize)
{
    return canAllocateCell(keySize, dataSize);
}

BTreePage::Index BTreePage::leafAdd(Key key, size_t dataSize)
{
    Index index = search(key);
    if(index < numCells() && keyCompare(cellKey(index), key) == 0) {
        return kInvalidIndex;
    } else {
        return insertCell(key, dataSize, index);
    }
}

void BTreePage::leafRemove(Index index)
{
    removeCell(index);
}

bool BTreePage::indirectCanAdd(size_t keySize)
{
    return canAllocateCell(keySize, sizeof(Page::Index));
}

void BTreePage::indirectAdd(Key key, BTreePage &childPage)
{
    if(!key) {
        BTreePage::Index newIndex = insertCell(key, sizeof(Page::Index), 0);
        Page::Index *indexData = reinterpret_cast<Page::Index*>(cellData(newIndex));
        *indexData = childPage.pageIndex();
        childPage.setParent(pageIndex());
    } else {
        Index index = search(key);
        if(index < numCells() && keyCompare(cellKey(index), key) == 0) {
            return;
        } else {
            BTreePage::Index newIndex = insertCell(key, sizeof(Page::Index), index);
            Page::Index *indexData = reinterpret_cast<Page::Index*>(cellData(newIndex));
            *indexData = childPage.pageIndex();
            childPage.setParent(pageIndex());
        }
    }
}

Page::Index BTreePage::indirectLookup(Key key)
{
    Index index = search(key);
    if(index == numCells() || keyCompare(cellKey(index), key) > 0) {
        index -= 1;
    }

    return indirectPageIndex(index);
}

Page::Index BTreePage::indirectPageIndex(Index index)
{
    Page::Index *indexData = reinterpret_cast<Page::Index*>(cellData(index));
    return *indexData;
}

void BTreePage::indirectRectifyDeficientChild(BTreePage &childPage)
{
    Index childIndex = search(childPage.cellKey(1)) - 1;

    if(childIndex < numCells() - 1) {
        BTreePage rightNeighbor = getPage(indirectPageIndex(childIndex + 1));
        if(rightNeighbor.canSupplyItem(0)) {
            indirectRotateLeft(childPage, rightNeighbor, childIndex);
        } else {
            indirectMergeChildren(childPage, rightNeighbor, childIndex + 1);
        }
    } else {
        BTreePage leftNeighbor = getPage(indirectPageIndex(childIndex - 1));
        if(leftNeighbor.canSupplyItem(leftNeighbor.numCells() - 1)) {
            indirectRotateRight(leftNeighbor, childPage, childIndex);
        } else {
            indirectMergeChildren(leftNeighbor, childPage, childIndex);
        }
    }
}

void BTreePage::indirectRotateRight(BTreePage &leftChild, BTreePage &rightChild, Index index)
{
    if(rightChild.type() == BTreePage::Type::Indirect) {
        BTreePage movedChild = getPage(leftChild.indirectPageIndex(leftChild.numCells() - 1));
        rightChild.indirectPushHead(cellKey(index), movedChild);
        setCellKey(index, leftChild.cellKey(leftChild.numCells() - 1));
        leftChild.indirectPopTail();
    } else {
        Size size = leftChild.cellDataSize(leftChild.numCells() - 1);
        BTreePage::Index dstIndex = rightChild.insertCell(leftChild.cellKey(leftChild.numCells() - 1), size, 0);
        BTreePage::Index srcIndex = leftChild.numCells() - 1;
        std::memcpy(rightChild.cellData(dstIndex), leftChild.cellData(srcIndex), size);
        setCellKey(index, leftChild.cellKey(leftChild.numCells() - 1));
        leftChild.removeCell(leftChild.numCells() - 1);
    }
}

void BTreePage::indirectRotateLeft(BTreePage &leftChild, BTreePage &rightChild, Index index)
{
    if(rightChild.type() == BTreePage::Type::Indirect) {
        BTreePage movedChild = getPage(rightChild.indirectPageIndex(0));
        leftChild.indirectPushTail(cellKey(index + 1), movedChild);
        setCellKey(index + 1, rightChild.cellKey(1));
        rightChild.indirectPopHead();
    } else {
        Size dataSize = rightChild.cellDataSize(0);
        BTreePage::Index dstIndex = leftChild.insertCell(rightChild.cellKey(0), dataSize, leftChild.numCells());
        BTreePage::Index srcIndex = 0;
        std::memcpy(leftChild.cellData(dstIndex), rightChild.cellData(srcIndex), dataSize);
        setCellKey(index + 1, rightChild.cellKey(1));
        rightChild.removeCell(0);
    }
}

void BTreePage::indirectMergeChildren(BTreePage &leftChild, BTreePage &rightChild, Index index)
{
    for(Index i=0; i<rightChild.numCells(); i++) {
        if(leftChild.type() == BTreePage::Type::Indirect) {
            BTreePage movedChild = getPage(rightChild.indirectPageIndex(i));
            if(i == 0) {
                leftChild.indirectPushTail(cellKey(index), movedChild);
            } else {
                leftChild.indirectPushTail(rightChild.cellKey(i), movedChild);
            }
        } else {
            Size dataSize = rightChild.cellDataSize(i);
            BTreePage::Index dstIndex = leftChild.insertCell(rightChild.cellKey(i), dataSize, leftChild.numCells());
            BTreePage::Index srcIndex = i;
            std::memcpy(leftChild.cellData(dstIndex), rightChild.cellData(srcIndex), dataSize);
        }
    }
    removeCell(index);
    if(rightChild.nextSibling() != Page::kInvalidIndex) {
        getPage(rightChild.nextSibling()).setPrevSibling(leftChild.pageIndex());
    }
    leftChild.setNextSibling(rightChild.nextSibling());
    BTreePage next = getPage(rightChild.nextSibling());
    pageSet().deletePage(rightChild.page());
}

void BTreePage::indirectPushHead(Key oldHeadKey, BTreePage &childPage)
{
    setCellKey(0, oldHeadKey);
    BTreePage::Index dstIndex = insertCell(Key(), sizeof(Page::Index), 0);
    *reinterpret_cast<Page::Index*>(cellData(dstIndex)) = childPage.pageIndex();
    childPage.setParent(pageIndex());
}

void BTreePage::indirectPushTail(Key key, BTreePage &childPage)
{
    if(numCells() == 0) {
        key = Key();
    }

    BTreePage::Index dstIndex = BTreePage::insertCell(key, sizeof(Page::Index), numCells());
    *reinterpret_cast<Page::Index*>(cellData(dstIndex)) = childPage.pageIndex();
    childPage.setParent(pageIndex());  
}
    
void BTreePage::indirectPopHead()
{
    setCellKey(1, Key());
    removeCell(0);
}

void BTreePage::indirectPopTail()
{
    removeCell(numCells() - 1);
}

BTreePage::Header &BTreePage::header()
{
    return *reinterpret_cast<Header*>(mPage.data(0));
}

uint16_t *BTreePage::offsetsArray()
{
    return reinterpret_cast<uint16_t*>(mPage.data(sizeof(Header)));
}

void *BTreePage::cell(Index index)
{
    if(index >= header().numCells) {
        return nullptr;
    }

    return reinterpret_cast<void*>(mPage.data(cellOffset(index)));
}

BTreePage::Size BTreePage::cellSize(Index index)
{
    return cellTotalKeySize(index) + cellTotalDataSize(index);
}

uint16_t BTreePage::cellOffset(Index index)
{ 
    return offsetsArray()[index];
}

uint16_t BTreePage::cellKeyOffset(Index index)
{
    uint16_t offset = cellOffset(index);
    if(mKeyDefinition.fixedSize() == 0) {
        offset += sizeof(uint16_t);
    }

    return offset;
}

uint16_t BTreePage::cellDataOffset(Index index)
{
    uint16_t offset = cellOffset(index);
    BTreePage::Size keySize = cellTotalKeySize(index);

    return offset + keySize + ((type() == Type::Leaf) ? sizeof(uint16_t) : 0);
}

uint16_t BTreePage::allocateCell(Key key, Size dataSize)
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

bool BTreePage::canAllocateCell(Size keySize, Size dataSize)
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

BTreePage::Index BTreePage::search(Key key)
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

int BTreePage::keyCompare(Key a, Key b)
{
    return mKeyDefinition.compare(a, b);
}

BTreePage BTreePage::getPage(Page::Index index)
{
    return BTreePage(pageSet().page(index), mKeyDefinition);
}

void BTreePage::defragPage()
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

PageSet &BTreePage::pageSet()
{
    return mPage.pageSet();
}