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

    struct Pointer {
        Page::Index pageIndex;
        BTreePage::Index cellIndex;
    };

    BTree(PageSet &pageSet, Page::Index rootIndex, std::unique_ptr<BTreePage::KeyDefinition> keyDefinition);

    void initialize();

    Pointer lookup(Key key);
    Pointer add(Key key, BTreePage::Size size);
    void remove(Pointer pointer);

    void *key(Pointer pointer);
    void *data(Pointer pointer);

    Pointer first();
    Pointer last();

    Pointer next(Pointer pointer);
    Pointer prev(Pointer pointer);

    template <typename F> void print(F printCell) {
        Page &page = mPageSet.page(mRootIndex);
        BTreePage(page, *mKeyDefinition).print("", printCell);
    }

private:
    PageSet &mPageSet;
    Page::Index mRootIndex;
    std::unique_ptr<BTreePage::KeyDefinition> mKeyDefinition;

    BTreePage findLeaf(BTreePage::Key);
    BTreePage getPage(Page::Index index);
    int keyCompare(Key a, Key b);
};

#endif