#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "CellPage.hpp"

class BTree {
public:
    typedef uint32_t RowId;
    static const RowId kInvalidRowId = UINT32_MAX;

    BTree(PageSet &pageSet, Page::Index rootIndex);

    void intialize();

    void *lookup(RowId rowId);
    void *add(RowId rowId, CellPage::Size size);
    void remove(RowId);

    void print();

private:
    PageSet &mPageSet;
    Page::Index mRootIndex;

    struct PageHeader {
        enum Type : uint8_t {
            Leaf,
            Indirect
        };

        Type type;
        Page::Index parent;
    };

    class LeafPage {
    public:
        LeafPage(Page &page);

        void initialize();

        CellPage &cellPage();
        Page &page();
        Page::Index parent();
        void setParent(Page::Index parent);

        RowId lowestRowId();

        void *lookup(RowId rowId);
        bool canAdd(size_t size);
        void *add(RowId rowId, size_t size);
        void remove(RowId rowId);

        LeafPage split(PageSet &pageSet);

        void print();

    private:
        PageHeader &header();

        CellPage::Index search(RowId rowId);
        RowId cellRowId(CellPage::Index index);

        CellPage mCellPage;
    };

    class IndirectPage
    {
    public:
        IndirectPage(Page &page);
    
        void initialize();

        CellPage &cellPage();
        Page &page();
        Page::Index parent();
        void setParent(Page::Index parent);

        RowId lowestRowId();

        bool canAdd();
        void add(RowId rowId, Page::Index pageIndex);

        Page::Index lookup(RowId rowId);
        IndirectPage split(PageSet &pageSet);

        Page::Index cellPageIndex(CellPage::Index index);

    private:
        PageHeader &header();

        CellPage::Index search(RowId rowId);
        RowId cellRowId(CellPage::Index index);

        struct IndirectCell {
            RowId rowId;
            Page::Index pageIndex;
        };

        CellPage mCellPage;
    };

    LeafPage findLeaf(RowId rowId);
    void setPageParent(Page::Index pageIndex, Page::Index parent);
};

#endif