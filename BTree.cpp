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

void *BTree::lookup(void *key)
{
    TreePage leafPage = findLeaf(key);
    return leafPage.leafLookup(key);
}

void *BTree::add(void *key, TreePage::Size keySize, TreePage::Size dataSize)
{
    TreePage leafPage = findLeaf(key);
    if(leafPage.leafCanAdd(keySize, dataSize)) {
        return leafPage.leafAdd(key, keySize, dataSize);
    }
    
    TreePage::Index splitIndex = leafPage.numCells() / 2;
    TreePage::KeyValue splitKey;
    splitKey.data.resize(leafPage.cellKeySize(splitIndex));
    std::memcpy(splitKey.data.data(), leafPage.cellKey(splitIndex), splitKey.data.size());
    TreePage newLeafPage = leafPage.split(splitIndex);

    void *ret;
    if(mKeyDefinition->compare(splitKey.data.data(), key) <= 0) {
        ret = newLeafPage.leafAdd(key, keySize, dataSize);
    } else {
        ret = leafPage.leafAdd(key, keySize, dataSize);
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

            indirectPage.indirectPushTail(nullptr, 0, leftSplitPage);
            indirectPage.indirectPushTail(splitKey.data.data(), splitKey.data.size(), rightSplitPage);

            mRootIndex = indirectPage.page().index();
            break;
        } else {
            TreePage indirectPage = getPage(parentPageIndex);
            if(indirectPage.indirectCanAdd(sizeof(RowId))) {
                indirectPage.indirectAdd(splitKey.data.data(), splitKey.data.size(), rightSplitPage);
                break;
            } else {
                splitIndex = indirectPage.numCells() / 2;
                TreePage::KeyValue indirectSplitKey;
                indirectSplitKey.data.resize(indirectPage.cellKeySize(splitIndex));
                std::memcpy(indirectSplitKey.data.data(), indirectPage.cellKey(splitIndex), indirectSplitKey.data.size());
                TreePage newIndirectPage  = indirectPage.split(splitIndex);

                if(mKeyDefinition->compare(indirectSplitKey.data.data(), splitKey.data.data()) <= 0) {
                    newIndirectPage.indirectAdd(splitKey.data.data(), splitKey.data.size(), rightSplitPage);
                } else {
                    indirectPage.indirectAdd(splitKey.data.data(), splitKey.data.size(), rightSplitPage);
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

void BTree::remove(void *key, size_t keySize)
{
    TreePage leafPage = findLeaf(key);
    leafPage.leafRemove(key);

    Page::Index index = leafPage.page().index();
    TreePage::KeyValue removedKey;
    removedKey.data.resize(keySize);
    std::memcpy(removedKey.data.data(), key, keySize);

    RowId removedRowId = *reinterpret_cast<RowId*>(key);
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
        removedKey = parentPage.indirectRectifyDeficientChild(page, removedKey.data.data());

        index = parentPage.page().index();
    }
}

TreePage BTree::findLeaf(void *key)
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