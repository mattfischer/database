#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "TreePage.hpp"

#include <memory>
#include <iostream>

class BTree {
public:
    typedef TreePage::Key Key;
    typedef TreePage::KeyDefinition KeyDefinition;

    struct Pointer {
        Page::Index pageIndex;
        TreePage::Index cellIndex;
    };

    BTree(PageSet &pageSet, Page::Index rootIndex, std::unique_ptr<TreePage::KeyDefinition> keyDefinition);

    void initialize();

    Pointer lookup(Key key);
    Pointer add(Key key, TreePage::Size size);
    void remove(Pointer pointer);

    void *key(Pointer pointer);
    void *data(Pointer pointer);

    Pointer first();
    Pointer last();

    Pointer next(Pointer pointer);
    Pointer prev(Pointer pointer);

    template <typename F> void print(F printCell) {
        Page &page = mPageSet.page(mRootIndex);
        TreePage(page, *mKeyDefinition).print("", printCell);
    }

private:
    PageSet &mPageSet;
    Page::Index mRootIndex;
    std::unique_ptr<TreePage::KeyDefinition> mKeyDefinition;

    TreePage findLeaf(TreePage::Key);
    TreePage getPage(Page::Index index);
    int keyCompare(Key a, Key b);
};

#endif