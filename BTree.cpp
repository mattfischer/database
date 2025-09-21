#include "BTree.hpp"

#include <iostream>

BTree::BTree(PageSet &pageSet, Page::Index rootIndex)
: mPageSet(pageSet)
{
    mRootIndex = rootIndex;
}

void BTree::intialize()
{
    TreePage leafPage(mPageSet.page(mRootIndex));
    leafPage.initialize(TreePage::Type::Leaf);
}

void *BTree::lookup(RowId rowId)
{
    TreePage leafPage = findLeaf(rowId);
    return leafPage.leafLookup(rowId);
}

void *BTree::add(RowId rowId, TreePage::Size size)
{
    TreePage leafPage = findLeaf(rowId);
    if(leafPage.leafCanAdd(size)) {
        return leafPage.leafAdd(rowId, size);
    }
    
    auto [newLeafPage, splitRow] = leafPage.split();

    void *ret;
    if(splitRow <= rowId) {
        ret = newLeafPage.leafAdd(rowId, size);
    } else {
        ret = leafPage.leafAdd(rowId, size);
    }

    Page::Index parentPageIndex = leafPage.parent();
    Page::Index leftSplitIndex = leafPage.page().index();
    Page::Index rightSplitIndex = newLeafPage.page().index();

    while(true) {
        TreePage leftSplitPage(mPageSet.page(leftSplitIndex));
        TreePage rightSplitPage(mPageSet.page(rightSplitIndex));

        if(parentPageIndex == Page::kInvalidIndex) {
            TreePage indirectPage(mPageSet.addPage());
            indirectPage.initialize(TreePage::Type::Indirect);

            indirectPage.indirectAdd(TreePage::kInvalidRowId, leftSplitPage);
            indirectPage.indirectAdd(splitRow, rightSplitPage);

            mRootIndex = indirectPage.page().index();
            break;
        } else {
            TreePage indirectPage(mPageSet.page(parentPageIndex));
            if(indirectPage.indirectCanAdd()) {
                indirectPage.indirectAdd(splitRow, rightSplitPage);
                break;
            } else {
                auto [newIndirectPage, indirectSplitRow] = indirectPage.split();

                if(indirectSplitRow <= splitRow) {
                    newIndirectPage.indirectAdd(splitRow, rightSplitPage);
                } else {
                    indirectPage.indirectAdd(splitRow, rightSplitPage);
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
    TreePage leafPage = findLeaf(rowId);
    leafPage.leafRemove(rowId);

    Page::Index index = leafPage.page().index();
    RowId removedRowId = rowId;
    while(true) {
        TreePage page(mPageSet.page(index));
        if(!page.isDeficient()) {
            break;
        }

        if(page.parent() == Page::kInvalidIndex) {
            if(page.type() == TreePage::Indirect && page.numCells() == 1) {
                mRootIndex = page.indirectPageIndex(0);
                mPageSet.deletePage(page.page());
                TreePage(mPageSet.page(mRootIndex)).setParent(Page::kInvalidIndex);
            }
            break;
        }
        TreePage parentPage(mPageSet.page(page.parent()));
        removedRowId = parentPage.indirectRectifyDeficientChild(page, removedRowId);

        index = parentPage.page().index();
    }
}

TreePage BTree::findLeaf(RowId rowId)
{
    Page::Index index = mRootIndex;
    while(true) {
        TreePage page(mPageSet.page(index));
        if(page.type() == TreePage::Type::Leaf) {
            break;
        }
        
        index = page.indirectLookup(rowId);
    }

    return TreePage(mPageSet.page(index));
}
