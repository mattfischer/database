#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "TreePage.hpp"

class BTree {
public:
    typedef TreePage::RowId RowId;

    BTree(PageSet &pageSet, Page::Index rootIndex);

    void intialize();

    void *lookup(RowId rowId);
    void *add(RowId rowId, TreePage::Size size);
    void remove(RowId);

    void print();

private:
    PageSet &mPageSet;
    Page::Index mRootIndex;

    TreePage findLeaf(RowId rowId);
};

#endif