#include "BTree.hpp"

#include <iostream>

BTree::BTree(PageSet &pageSet, Page::Index rootIndex, std::unique_ptr<TreePage::KeyDefinition> keyDefinition)
: mPageSet(pageSet)
, mKeyDefinition(std::move(keyDefinition))
{
    mRootIndex = rootIndex;
}

void BTree::intialize()
{
    TreePage leafPage(mPageSet.page(mRootIndex), *mKeyDefinition);
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
    RowId splitRow = leafPage.cellRowId(splitIndex);
    TreePage newLeafPage = leafPage.split(splitIndex);

    void *ret;
    if(mKeyDefinition->compare(&splitRow, key) <= 0) {
        ret = newLeafPage.leafAdd(key, keySize, dataSize);
    } else {
        ret = leafPage.leafAdd(key, keySize, dataSize);
    }

    Page::Index parentPageIndex = leafPage.parent();
    Page::Index leftSplitIndex = leafPage.page().index();
    Page::Index rightSplitIndex = newLeafPage.page().index();

    while(true) {
        TreePage leftSplitPage(mPageSet.page(leftSplitIndex), *mKeyDefinition);
        TreePage rightSplitPage(mPageSet.page(rightSplitIndex), *mKeyDefinition);

        if(parentPageIndex == Page::kInvalidIndex) {
            TreePage indirectPage(mPageSet.addPage(), *mKeyDefinition);
            indirectPage.initialize(TreePage::Type::Indirect);

            indirectPage.indirectPushTail(nullptr, 0, leftSplitPage);
            indirectPage.indirectPushTail(&splitRow, sizeof(RowId), rightSplitPage);

            mRootIndex = indirectPage.page().index();
            break;
        } else {
            TreePage indirectPage(mPageSet.page(parentPageIndex), *mKeyDefinition);
            if(indirectPage.indirectCanAdd()) {
                indirectPage.indirectAdd(&splitRow, sizeof(RowId), rightSplitPage);
                break;
            } else {
                splitIndex = indirectPage.numCells() / 2;
                RowId indirectSplitRow = indirectPage.cellRowId(splitIndex);
                TreePage newIndirectPage  = indirectPage.split(splitIndex);

                if(indirectSplitRow <= splitRow) {
                    newIndirectPage.indirectAdd(&splitRow, sizeof(RowId), rightSplitPage);
                } else {
                    indirectPage.indirectAdd(&splitRow, sizeof(RowId), rightSplitPage);
                }

                parentPageIndex = indirectPage.parent();
                leftSplitIndex = indirectPage.page().index();
                rightSplitIndex = newIndirectPage.page().index();
                splitRow = indirectSplitRow;
            }
        }
    }

    return ret;
}

void BTree::remove(void *key)
{
    TreePage leafPage = findLeaf(key);
    leafPage.leafRemove(key);

    Page::Index index = leafPage.page().index();
    RowId removedRowId = *reinterpret_cast<RowId*>(key);
    while(true) {
        TreePage page(mPageSet.page(index), *mKeyDefinition);
        if(!page.isDeficient()) {
            break;
        }

        if(page.parent() == Page::kInvalidIndex) {
            if(page.type() == TreePage::Indirect && page.numCells() == 1) {
                mRootIndex = page.indirectPageIndex(0);
                mPageSet.deletePage(page.page());
                TreePage(mPageSet.page(mRootIndex), *mKeyDefinition).setParent(Page::kInvalidIndex);
            }
            break;
        }
        TreePage parentPage(mPageSet.page(page.parent()), *mKeyDefinition);
        removedRowId = parentPage.indirectRectifyDeficientChild(page, &removedRowId);

        index = parentPage.page().index();
    }
}

TreePage BTree::findLeaf(void *key)
{
    Page::Index index = mRootIndex;
    while(true) {
        TreePage page(mPageSet.page(index), *mKeyDefinition);
        if(page.type() == TreePage::Type::Leaf) {
            break;
        }
        
        index = page.indirectLookup(key);
    }

    return TreePage(mPageSet.page(index), *mKeyDefinition);
}
