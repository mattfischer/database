#ifndef CELLPAGE_HPP
#define CELLPAGE_HPP

#include "Page.hpp"

class CellPage {
public:
    typedef uint16_t Index;
    typedef uint16_t Size;

    CellPage(Page &page, Size fixedCellSize = 0);

    Index numCells();
    void *cell(Index index);

    Index addCell(Size size);

    void insertCell(Size size, Index index);
    void shiftLeftAndInsertCell(Size size, Index index);
    void shiftRightAndInsertCell(Size size, Index index);
    
    void removeCell(Index index);
    void removeCells(Index begin, Index end);

private:
    struct Header {
        uint16_t numCells;
        uint16_t dataStart;
    };

    Header &header();
    uint16_t *offsetsArray();

    uint16_t cellOffset(Index index);
    uint16_t cellDataOffset(Index index);

    uint16_t allocateCell(Size size);

    void defragPage();

    Page &mPage;
    Size mFixedCellSize;
};
#endif