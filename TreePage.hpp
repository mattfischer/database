#ifndef TREEPAGE_HPP
#define TREEPAGE_HPP

#include "PageSet.hpp"

#include <string>

class TreePage {
public:
    typedef uint32_t RowId;
    static const RowId kInvalidRowId = UINT32_MAX;

    typedef uint16_t Index;
    typedef uint16_t Size;

    enum Type : uint8_t {
        Leaf,
        Indirect
    };

    TreePage(Page &page);

    void initialize(Type type);

    Page &page();
    Page::Index pageIndex();

    Type type();

    Page::Index parent();
    void setParent(Page::Index parent);

    Size freeSpace();

    Index numCells();

    void *cellData(Index index);
    Size cellDataSize(Index index);

    RowId cellRowId(Index index);
    void setCellRowId(Index index, RowId rowId);

    void *insertCell(RowId rowId, Size size, Index index);

    void removeCell(Index index);
    void removeCells(Index begin, Index end);

    void copyCells(TreePage &page, Index begin, Index end);

    std::tuple<TreePage, RowId> split();

    bool isDeficient();
    bool canSupplyItem();

    void *leafLookup(RowId rowId);
    bool leafCanAdd(size_t size);
    void *leafAdd(RowId rowId, size_t size);
    void leafRemove(RowId rowId);

    bool indirectCanAdd();
    void indirectAdd(RowId rowId, TreePage &childPage);
    Page::Index indirectPageIndex(Index index);
    Page::Index indirectLookup(RowId rowId);
    RowId indirectRectifyDeficientChild(TreePage &childPage, RowId removedRowId);

    void print(const std::string &prefix);

private:
    struct Header {
        Type type;
        uint16_t numCells;
        uint16_t dataStart;
        uint16_t freeSpace;
        Page::Index parent;
    };

    Header &header();
    uint16_t *offsetsArray();

    void *cell(Index index);
    Size cellSize(Index index);
    uint16_t cellOffset(Index index);
    uint16_t cellDataOffset(Index index);

    uint16_t allocateCell(RowId rowId, Size size);
    bool canAllocateCell(Size size);

    Index search(RowId rowId);

    void defragPage();

    PageSet &pageSet();

    Page &mPage;
};

#endif