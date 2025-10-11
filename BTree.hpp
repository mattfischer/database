#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "TreePage.hpp"

#include <memory>
#include <iostream>

class BTree {
public:
    typedef uint32_t RowId;
    typedef TreePage::Key Key;

    class RowIdKeyDefinition : public TreePage::KeyDefinition {
    public:
        virtual TreePage::Size fixedSize() override { return sizeof(RowId); }
        virtual int compare(Key a, Key b) override {
            RowId ar = *reinterpret_cast<RowId*>(a.data);
            RowId br = *reinterpret_cast<RowId*>(b.data);
            if(ar < br) return -1;
            if(ar == br) return 0;
            return 1;
        }
        virtual void print(Key key) override {
            std::cout << *reinterpret_cast<RowId*>(key.data);
        }
    };

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
};

#endif