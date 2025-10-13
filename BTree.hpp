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

    BTree(PageSet &pageSet, Page::Index rootIndex, std::unique_ptr<TreePage::KeyDefinition> keyDefinition);

    void initialize();

    void *lookup(Key key);
    void *add(Key key, TreePage::Size size);
    void remove(Key key);

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