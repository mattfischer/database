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

void *BTree::add(RowId rowId, TreePage::Size size)
{
    TreePage leafPage = findLeaf(&rowId);
    if(leafPage.leafCanAdd(sizeof(RowId), size)) {
        return leafPage.leafAdd(&rowId, sizeof(RowId), size);
    }
    
    auto [newLeafPage, splitRow] = leafPage.split();

    void *ret;
    if(splitRow <= rowId) {
        ret = newLeafPage.leafAdd(&rowId, sizeof(RowId), size);
    } else {
        ret = leafPage.leafAdd(&rowId, sizeof(RowId), size);
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
                auto [newIndirectPage, indirectSplitRow] = indirectPage.split();

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

void BTree::remove(RowId rowId)
{
    TreePage leafPage = findLeaf(&rowId);
    leafPage.leafRemove(rowId);

    Page::Index index = leafPage.page().index();
    RowId removedRowId = rowId;
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
        removedRowId = parentPage.indirectRectifyDeficientChild(page, removedRowId);

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
