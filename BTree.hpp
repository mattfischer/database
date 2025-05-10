#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "CellPage.hpp"

class BTree {
public:
    BTree(PageSet &pageSet);

private:
    PageSet &mPageSet;
    Page::Index mRoot;

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

        Page &page();

    private:
        PageHeader &header();

        CellPage mCellPage;
    };

    LeafPage addLeafPage();
};

#endif