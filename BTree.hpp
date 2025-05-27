#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "CellPage.hpp"

class BTree {
public:
    typedef uint32_t RowId;

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
            Leaf
        };

        Type type;
    };

    class LeafPage {
    public:
        LeafPage(Page &page);

        void initialize();

        CellPage &cellPage();
        Page &page();

        void *lookup(RowId rowId);
        void *add(RowId rowId, size_t size);
        void remove(RowId rowId);

        void print();

    private:
        PageHeader &header();

        CellPage::Index search(RowId rowId);
        RowId cellRowId(CellPage::Index index);

        CellPage mCellPage;
    };
};

#endif