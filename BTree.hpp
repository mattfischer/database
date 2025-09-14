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

    class TreePage : public CellPage {
    public:
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

    protected:
        CellPage::Index search(RowId rowId);
        RowId cellRowId(CellPage::Index index);
        void *cellData(CellPage::Index index);

        void *insertCell(RowId rowId, CellPage::Size size, CellPage::Index index);
        bool canAllocateCell(CellPage::Size size);
        TreePage split(PageSet &pageSet);

    private:
        struct Header {
            Type type;
            Page::Index parent;
        };
 
        Header &header();
    };

    class LeafPage : public TreePage {
    public:
        LeafPage(Page &page);

        void initialize();

        void *lookup(RowId rowId);
        bool canAdd(size_t size);
        void *add(RowId rowId, size_t size);
        void remove(RowId rowId);

        LeafPage split(PageSet &pageSet);

        void print();
    };

    class IndirectPage : public TreePage
    {
    public:
        IndirectPage(Page &page);
    
        void initialize();

        bool canAdd();
        void add(TreePage &page);

        Page::Index lookup(RowId rowId);
        IndirectPage split(PageSet &pageSet);

        Page::Index cellPageIndex(CellPage::Index index);
    };

    LeafPage findLeaf(RowId rowId);
};

#endif