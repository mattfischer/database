#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "TreePage.hpp"

#include <memory>

class BTree {
public:
    typedef TreePage::RowId RowId;

    class RowIdKeyDefinition : public TreePage::KeyDefinition {
    public:
        virtual TreePage::Size fixedSize() override { return sizeof(RowId); }
        virtual int compare(void *a, void *b) {
            RowId ar = *reinterpret_cast<RowId*>(a);
            RowId br = *reinterpret_cast<RowId*>(b);
            if(ar < br) return -1;
            if(ar == br) return 0;
            return 1;
        }
    };

    BTree(PageSet &pageSet, Page::Index rootIndex, std::unique_ptr<TreePage::KeyDefinition> keyDefinition);

    void intialize();

    void *lookup(void *key);
    void *add(void *key, TreePage::Size keySize, TreePage::Size size);
    void remove(void *key);

    template <typename F> void print(F printCell) {
        Page &page = mPageSet.page(mRootIndex);
        TreePage(page, *mKeyDefinition).print("", printCell);
    }

private:
    PageSet &mPageSet;
    Page::Index mRootIndex;
    std::unique_ptr<TreePage::KeyDefinition> mKeyDefinition;

    TreePage findLeaf(void *key);
};

#endif