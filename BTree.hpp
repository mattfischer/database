#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "CellPage.hpp"
#include "TreePage.hpp"
#include "LeafPage.hpp"

class BTree {
public:
    typedef TreePage::RowId RowId;

    BTree(PageSet &pageSet, Page::Index rootIndex);

    void intialize();

    void *lookup(RowId rowId);
    void *add(RowId rowId, CellPage::Size size);
    void remove(RowId);

    void print();

private:
    PageSet &mPageSet;
    Page::Index mRootIndex;

    LeafPage findLeaf(RowId rowId);
};

#endif