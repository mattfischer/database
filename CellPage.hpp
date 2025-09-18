#ifndef CELLPAGE_HPP
#define CELLPAGE_HPP

#include "Page.hpp"

class CellPage {
public:
    typedef uint16_t Index;
    typedef uint16_t Size;

    CellPage(Page &page, size_t extraHeaderSize, Size fixedCellSize = 0);
    void initialize();

    Index numCells();
    void *cell(Index index);
    void *extraHeader();
    Page &page();
    Size fixedCellSize();

    Index addCell(Size size);

    void insertCell(Size size, Index index);
    void shiftLeftAndInsertCell(Size size, Index index);
    void shiftRightAndInsertCell(Size size, Index index);
    
    void removeCell(Index index);
    void removeCells(Index begin, Index end);

    void copyCells(CellPage &page, Index begin, Index end);

    bool canAllocateCell(Size size);

    uint16_t cellSize(Index index);

    void defragPage();
    Size freeSpace();

    void print();

private:
    struct Header {
        uint16_t numCells;
        uint16_t dataStart;
        uint16_t freeSpace;
    };

    Header &header();
    uint16_t *offsetsArray();

    uint16_t cellOffset(Index index);
    uint16_t cellDataOffset(Index index);

    uint16_t allocateCell(Size size);

    Page &mPage;
    Size mFixedCellSize;
    size_t mExtraHeaderSize;
};
#endif