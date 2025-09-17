#ifndef TREEPAGE_HPP
#define TREEPAGE_HPP

#include "CellPage.hpp"
#include "PageSet.hpp"

#include <tuple>

class TreePage : public CellPage {
public:
    typedef uint32_t RowId;
    static const RowId kInvalidRowId = UINT32_MAX;

    TreePage(Page &page, Size fixedCellSize = 0);

    enum Type : uint8_t {
        Leaf,
        Indirect
    };

    void initialize(Type type);

    Type type();

    Page::Index parent();
    void setParent(Page::Index parent);
  
    static Type pageType(Page &page);
    static void printPage(Page &page);

protected:
    CellPage::Index search(RowId rowId);
    RowId cellRowId(CellPage::Index index);
    void setCellRowId(CellPage::Index index, RowId rowId);
    void *cellData(CellPage::Index index);

    void *insertCell(RowId rowId, CellPage::Size size, CellPage::Index index);
    bool canAllocateCell(CellPage::Size size);
    RowId split(TreePage &newPage);

private:
    struct Header {
        Type type;
        Page::Index parent;
    };

    Header &header();
};

#endif