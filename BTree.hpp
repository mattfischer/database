#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "BTreePage.hpp"

#include <memory>
#include <iostream>

class BTree {
public:
    typedef BTreePage::Key Key;
    typedef BTreePage::KeyDefinition KeyDefinition;
    typedef BTreePage::DataDefinition DataDefinition;

    struct Pointer {
        Page::Index pageIndex;
        BTreePage::Index cellIndex;
    };

    BTree(PageSet &pageSet, Page::Index rootIndex, std::unique_ptr<KeyDefinition> keyDefinition, std::unique_ptr<DataDefinition> dataDefinition);

    void initialize();

    PageSet &pageSet();

    Pointer lookup(Key key);
    Pointer add(Key key, BTreePage::Size size);
    void remove(Pointer pointer);

    void *key(Pointer pointer);
    void *data(Pointer pointer);

    Pointer first();
    Pointer last();

    Pointer next(Pointer pointer);
    Pointer prev(Pointer pointer);

    void print();

private:
    PageSet &mPageSet;
    Page::Index mRootIndex;
    std::unique_ptr<KeyDefinition> mKeyDefinition;
    std::unique_ptr<DataDefinition> mDataDefinition;

    BTreePage findLeaf(Key);
    BTreePage getPage(Page::Index index);
    int keyCompare(Key a, Key b);
};

#endif