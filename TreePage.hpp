#ifndef TREEPAGE_HPP
#define TREEPAGE_HPP

#include "CellPage.hpp"
#include "PageSet.hpp"

class TreePage : public CellPage {
public:
    typedef uint32_t RowId;

    TreePage(Page &page, Size fixedCellSize = 0);

    enum Type : uint8_t {
        Leaf,
        Indirect
    };

    void initialize(Type type);

    Type type();

    Page::Index parent();
    void setParent(Page::Index parent);

    RowId lowestRowId();
    void updateRowId(RowId oldRowId, RowId newRowId);

    static Type pageType(Page &page);

protected:
    CellPage::Index search(RowId rowId);
    RowId cellRowId(CellPage::Index index);
    void *cellData(CellPage::Index index);

    void *insertCell(RowId rowId, CellPage::Size size, CellPage::Index index);
    bool canAllocateCell(CellPage::Size size);
    TreePage split();

private:
    struct Header {
        Type type;
        Page::Index parent;
    };

    Header &header();
};

#endif