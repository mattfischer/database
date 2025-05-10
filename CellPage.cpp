#include "CellPage.hpp"

#include <algorithm>

CellPage::CellPage(Page &page, size_t extraHeaderSize, Size fixedCellSize)
 : mPage(page)
{
    mFixedCellSize = fixedCellSize;
    mExtraHeaderSize = extraHeaderSize;
}

void CellPage::initialize()
{
    Header &head = header();
    head.numCells = 0;
    head.dataStart = mPage.size();
}

CellPage::Index CellPage::numCells()
{
    return header().numCells;
}

void *CellPage::cell(Index index)
{
    if(index > header().numCells) {
        return nullptr;
    }

    return reinterpret_cast<void*>(mPage.data(cellDataOffset(index)));
}

void *CellPage::extraHeader()
{
    return reinterpret_cast<void*>(mPage.data(sizeof(Header)));
}

Page &CellPage::page()
{
    return mPage;
}

CellPage::Index CellPage::addCell(Size size)
{
    Header &head = header();
    Index index = head.numCells;

    insertCell(size, index);
    return index;
}

void CellPage::insertCell(Size size, Index index)
{
    Header &head = header();
    uint16_t *offsets = offsetsArray();

    uint16_t offset = allocateCell(size);
    std::memmove(offsets + index + 1, offsets + index, (head.numCells - index) * sizeof(uint16_t));
    offsets[index] = offset;
    head.numCells++;
}

void CellPage::shiftLeftAndInsertCell(Size size, Index index)
{
    Header &head = header();
    uint16_t *offsets = offsetsArray();

    uint16_t offset = allocateCell(size);
    std::memmove(offsets, offsets + 1, index * sizeof(uint16_t));
    offsets[index] = offset;
}

void CellPage::shiftRightAndInsertCell(Size size, Index index)
{
    Header &head = header();
    uint16_t *offsets = offsetsArray();

    uint16_t offset = allocateCell(size);
    std::memmove(offsets + index + 1, offsets + index, (head.numCells - index - 1) * sizeof(uint16_t));
    offsets[index] = offset;
}

void CellPage::removeCell(Index index)
{
    removeCells(index, index + 1);
}

void CellPage::removeCells(Index begin, Index end)
{
    Header &head = header();
    uint16_t *array = offsetsArray();
    std::memmove(array + begin, array + end, (head.numCells - end) * sizeof(uint16_t));
    head.numCells -= (end - begin);
}

CellPage::Header &CellPage::header()
{
    return *reinterpret_cast<Header*>(mPage.data(0));
}

uint16_t *CellPage::offsetsArray()
{
    return reinterpret_cast<uint16_t*>(mPage.data(sizeof(Header) + mExtraHeaderSize));
}

uint16_t CellPage::cellOffset(Index index)
{ 
    return offsetsArray()[index];
}

uint16_t CellPage::cellDataOffset(Index index)
{
    uint16_t offset = cellOffset(index);
    return offset + (mFixedCellSize == 0) ? sizeof(uint16_t) : 0;
}

uint16_t CellPage::allocateCell(Size size)
{
    Header &head = header();
    uint16_t arrayEnd = sizeof(Header) + mExtraHeaderSize + (head.numCells + 1) * sizeof(uint16_t);

    Size totalSize = size + (mFixedCellSize == 0) ? sizeof(uint16_t) : 0;

    if(head.dataStart - arrayEnd < totalSize) {
        defragPage();
    }

    uint16_t offset = head.dataStart - totalSize;
    head.dataStart -= totalSize;
    if(mFixedCellSize == 0) {
        *reinterpret_cast<uint16_t*>(mPage.data(offset)) = totalSize;
    }

    return offset;
}

void CellPage::defragPage()
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
        Size size = mFixedCellSize;
        if(size == 0) {
            size = *reinterpret_cast<uint16_t*>(mPage.data(offsets[i]));
        }
        
        head.dataStart -= size;
        std::memmove(mPage.data(head.dataStart), mPage.data(offsets[i]), size);
        offsets[i] = head.dataStart;
    }
}