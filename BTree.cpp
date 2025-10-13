#include "BTree.hpp"

#include <iostream>

BTree::BTree(PageSet &pageSet, Page::Index rootIndex, std::unique_ptr<TreePage::KeyDefinition> keyDefinition)
: mPageSet(pageSet)
, mKeyDefinition(std::move(keyDefinition))
{
    mRootIndex = rootIndex;
}

void BTree::initialize()
{
    TreePage leafPage = getPage(mRootIndex);
    leafPage.initialize(TreePage::Type::Leaf);
}

void *BTree::lookup(Key key)
{
    TreePage leafPage = findLeaf(key);
    return leafPage.leafLookup(key);
}

struct KeyValue {
    KeyValue() = default;
    KeyValue(BTree::Key key) {
        data.resize(key.size);
        std::memcpy(data.data(), key.data, key.size);
    }

    operator BTree::Key() {
        return BTree::Key(data.data(), data.size());
    }

    std::vector<uint8_t> data;
};

void *BTree::add(Key key, TreePage::Size dataSize)
{
    TreePage leafPage = findLeaf(key);
    if(leafPage.leafCanAdd(key.size, dataSize)) {
        return leafPage.leafAdd(key, dataSize);
    }
    
    TreePage::Index splitIndex = leafPage.numCells() / 2;
    KeyValue splitKey = leafPage.cellKey(splitIndex);
    TreePage newLeafPage = leafPage.split(splitIndex);

    void *ret;
    if(keyCompare(splitKey, key) <= 0) {
        ret = newLeafPage.leafAdd(key, dataSize);
    } else {
        ret = leafPage.leafAdd(key, dataSize);
    }

    Page::Index parentPageIndex = leafPage.parent();
    Page::Index leftSplitIndex = leafPage.page().index();
    Page::Index rightSplitIndex = newLeafPage.page().index();

    while(true) {
        TreePage leftSplitPage = getPage(leftSplitIndex);
        TreePage rightSplitPage = getPage(rightSplitIndex);

        if(parentPageIndex == Page::kInvalidIndex) {
            TreePage indirectPage(mPageSet.addPage(), *mKeyDefinition);
            indirectPage.initialize(TreePage::Type::Indirect);

            indirectPage.indirectPushTail(Key(), leftSplitPage);
            indirectPage.indirectPushTail(splitKey, rightSplitPage);

            mRootIndex = indirectPage.page().index();
            break;
        } else {
            TreePage indirectPage = getPage(parentPageIndex);
            if(indirectPage.indirectCanAdd(splitKey.data.size())) {
                indirectPage.indirectAdd(splitKey, rightSplitPage);
                break;
            } else {
                splitIndex = indirectPage.numCells() / 2;
                KeyValue indirectSplitKey = indirectPage.cellKey(splitIndex);
                TreePage newIndirectPage = indirectPage.split(splitIndex);

                if(keyCompare(indirectSplitKey, splitKey) <= 0) {
                    newIndirectPage.indirectAdd(splitKey, rightSplitPage);
                } else {
                    indirectPage.indirectAdd(splitKey, rightSplitPage);
                }

                parentPageIndex = indirectPage.parent();
                leftSplitIndex = indirectPage.page().index();
                rightSplitIndex = newIndirectPage.page().index();
                splitKey = std::move(indirectSplitKey);
            }
        }
    }

    return ret;
}

void BTree::remove(Key key)
{
    TreePage leafPage = findLeaf(key);
    leafPage.leafRemove(key);

    Page::Index index = leafPage.page().index();

    while(true) {
        TreePage page = getPage(index);
        if(!page.isDeficient()) {
            break;
        }

        if(page.parent() == Page::kInvalidIndex) {
            if(page.type() == TreePage::Indirect && page.numCells() == 1) {
                mRootIndex = page.indirectPageIndex(0);
                mPageSet.deletePage(page.page());
                getPage(mRootIndex).setParent(Page::kInvalidIndex);
            }
            break;
        }
        TreePage parentPage = getPage(page.parent());
        parentPage.indirectRectifyDeficientChild(page);

        index = parentPage.page().index();
    }
}

TreePage BTree::findLeaf(Key key)
{
    Page::Index index = mRootIndex;
    while(true) {
        TreePage page = getPage(index);
        if(page.type() == TreePage::Type::Leaf) {
            break;
        }
        
        index = page.indirectLookup(key);
    }

    return getPage(index);
}

TreePage BTree::getPage(Page::Index index)
{
    return TreePage(mPageSet.page(index), *mKeyDefinition);
}

int BTree::keyCompare(Key a, Key b)
{
    return mKeyDefinition->compare(a, b);
}
