#include "BTree.hpp"

#include "IndirectPage.hpp"

#include <iostream>

BTree::BTree(PageSet &pageSet, Page::Index rootIndex)
: mPageSet(pageSet)
{
    mRootIndex = rootIndex;
}

void BTree::intialize()
{
    LeafPage leafPage(mPageSet.page(mRootIndex));
    leafPage.initialize();
}

void *BTree::lookup(RowId rowId)
{
    LeafPage leafPage = findLeaf(rowId);
    return leafPage.lookup(rowId);
}

void *BTree::add(RowId rowId, CellPage::Size size)
{
    LeafPage leafPage = findLeaf(rowId);
    if(leafPage.canAdd(size)) {
        return leafPage.add(rowId, size);
    }
    
    LeafPage newLeafPage = leafPage.split();

    void *ret;
    if(newLeafPage.lowestRowId() < rowId) {
        ret = newLeafPage.add(rowId, size);
    } else {
        ret = leafPage.add(rowId, size);
    }

    Page::Index parentPageIndex = leafPage.parent();
    Page::Index leftSplitIndex = leafPage.page().index();
    Page::Index rightSplitIndex = newLeafPage.page().index();

    while(true) {
        TreePage leftSplitPage(mPageSet.page(leftSplitIndex));
        TreePage rightSplitPage(mPageSet.page(rightSplitIndex));

        if(parentPageIndex == Page::kInvalidIndex) {
            IndirectPage indirectPage(mPageSet.addPage());
            indirectPage.initialize();

            indirectPage.add(leftSplitPage);
            indirectPage.add(rightSplitPage);

            mRootIndex = indirectPage.page().index();
            break;
        } else {
            IndirectPage indirectPage(mPageSet.page(parentPageIndex));
            if(indirectPage.canAdd()) {
                indirectPage.add(rightSplitPage);
                break;
            } else {
                IndirectPage newIndirectPage = indirectPage.split();

                if(newIndirectPage.lowestRowId() < rightSplitPage.lowestRowId()) {
                    newIndirectPage.add(rightSplitPage);
                } else {
                    indirectPage.add(rightSplitPage);
                }

                parentPageIndex = indirectPage.parent();
                leftSplitIndex = indirectPage.page().index();
                rightSplitIndex = newIndirectPage.page().index();
            }
        }
    }

    return ret;
}

void BTree::remove(RowId rowId)
{
    LeafPage leafPage = findLeaf(rowId);
    leafPage.remove(rowId);
}

void BTree::print()
{
    LeafPage leafPage(mPageSet.page(mRootIndex));
    leafPage.print();
}

LeafPage BTree::findLeaf(RowId rowId)
{
    Page::Index index = mRootIndex;
    while(true) {
        Page &page = mPageSet.page(index);
        if(TreePage::pageType(page) == TreePage::Type::Leaf) {
            break;
        }

        IndirectPage indirectPage(page);
        index = indirectPage.lookup(rowId);
    }

    return LeafPage(mPageSet.page(index));
}
