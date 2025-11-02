#include "BTree.hpp"

#include <iostream>

BTree::BTree(PageSet &pageSet, Page::Index rootIndex, std::unique_ptr<KeyDefinition> keyDefinition, std::unique_ptr<DataDefinition> dataDefinition)
: mPageSet(pageSet)
, mKeyDefinition(std::move(keyDefinition))
, mDataDefinition(std::move(dataDefinition))
{
    mRootIndex = rootIndex;
}

void BTree::initialize()
{
    BTreePage leafPage = getPage(mRootIndex);
    leafPage.initialize(BTreePage::Type::Leaf);
}

PageSet &BTree::pageSet()
{
    return mPageSet;
}

BTree::Pointer BTree::lookup(Key key, SearchComparison comparison, SearchPosition position)
{
    BTreePage leafPage = findLeaf(key, comparison, position);
    BTreePage::Index index = leafPage.leafLookup(key, comparison, position);
    if(index == BTreePage::kInvalidIndex) {
        return {Page::kInvalidIndex, 0};
    } else {
        return {leafPage.pageIndex(), index};
    }
}

BTree::Pointer BTree::add(Key key, BTreePage::Size dataSize)
{
    BTreePage leafPage = findLeaf(key, SearchComparison::GreaterThan, SearchPosition::First);
    if(leafPage.leafCanAdd(key.size, dataSize)) {
        return {leafPage.pageIndex(), leafPage.leafAdd(key, dataSize)};
    }
    
    BTreePage::Index splitIndex = leafPage.numCells() / 2;
    KeyValue splitKey = leafPage.cellKey(splitIndex);
    BTreePage newLeafPage = leafPage.split(splitIndex);

    Pointer ret;
    if(keyCompare(splitKey, key) <= 0) {
        ret = {newLeafPage.pageIndex(), newLeafPage.leafAdd(key, dataSize)};
    } else {
        ret = {leafPage.pageIndex(), leafPage.leafAdd(key, dataSize)};
    }

    Page::Index parentPageIndex = leafPage.parent();
    Page::Index leftSplitIndex = leafPage.page().index();
    Page::Index rightSplitIndex = newLeafPage.page().index();

    while(true) {
        BTreePage leftSplitPage = getPage(leftSplitIndex);
        BTreePage rightSplitPage = getPage(rightSplitIndex);

        if(parentPageIndex == Page::kInvalidIndex) {
            BTreePage indirectPage(mPageSet.addPage(), *mKeyDefinition, *mDataDefinition);
            indirectPage.initialize(BTreePage::Type::Indirect);

            indirectPage.indirectPushTail(Key(), leftSplitPage);
            indirectPage.indirectPushTail(splitKey, rightSplitPage);

            mRootIndex = indirectPage.page().index();
            break;
        } else {
            BTreePage indirectPage = getPage(parentPageIndex);
            if(indirectPage.indirectCanAdd(splitKey.data.size())) {
                indirectPage.indirectAdd(splitKey, rightSplitPage);
                break;
            } else {
                splitIndex = indirectPage.numCells() / 2;
                KeyValue indirectSplitKey = indirectPage.cellKey(splitIndex);
                BTreePage newIndirectPage = indirectPage.split(splitIndex);

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

bool BTree::resize(Pointer pointer, BTreePage::Size dataSize)
{
    BTreePage leafPage = getPage(pointer.pageIndex);
    return leafPage.leafResize(pointer.cellIndex, dataSize);
}

void BTree::remove(Pointer pointer, std::span<Pointer*> trackPointers)
{
    BTreePage leafPage = getPage(pointer.pageIndex);
    leafPage.leafRemove(pointer.cellIndex, trackPointers);
    Page::Index index = leafPage.page().index();

    while(true) {
        BTreePage page = getPage(index);
        if(!page.isDeficient()) {
            break;
        }

        if(page.parent() == Page::kInvalidIndex) {
            if(page.type() == BTreePage::Indirect && page.numCells() == 1) {
                mRootIndex = page.indirectPageIndex(0);
                mPageSet.deletePage(page.page());
                getPage(mRootIndex).setParent(Page::kInvalidIndex);
            }
            break;
        }
        BTreePage parentPage = getPage(page.parent());
        parentPage.indirectRectifyDeficientChild(page, trackPointers);

        index = parentPage.page().index();
    }
}

void *BTree::key(Pointer pointer)
{
    if(pointer.pageIndex == Page::kInvalidIndex) {
        return nullptr;
    } else {
        BTreePage page = getPage(pointer.pageIndex);
        
        return page.cellKey(pointer.cellIndex).data;    
    }
}

void *BTree::data(Pointer pointer)
{
    if(pointer.pageIndex == Page::kInvalidIndex) {
        return nullptr;
    } else {
        BTreePage page = getPage(pointer.pageIndex);
        return page.cellData(pointer.cellIndex);
    }
}

BTree::Pointer BTree::first()
{
    Page::Index index = mRootIndex;
    while(true) {
        BTreePage page = getPage(index);
        if(page.type() == BTreePage::Type::Leaf) {
            break;
        }
        
        index = page.indirectPageIndex(0);
    }

    return {index, 0};
}

BTree::Pointer BTree::last()
{
    Page::Index index = mRootIndex;
    while(true) {
        BTreePage page = getPage(index);
        if(page.type() == BTreePage::Type::Leaf) {
            break;
        }
        
        index = page.indirectPageIndex(page.numCells() - 1);
    }

    BTreePage page = getPage(index);
    return {index, (BTreePage::Index)(page.numCells() - 1)};
}

bool BTree::moveNext(Pointer &pointer)
{
    BTreePage page = getPage(pointer.pageIndex);
    if(pointer.cellIndex < page.numCells() - 1) {
        pointer = {pointer.pageIndex, (BTreePage::Index)(pointer.cellIndex + 1)};
        return true;
    } else {
        pointer = {page.nextSibling(), 0};
        return pointer.valid();
    }
}

bool BTree::movePrev(Pointer &pointer)
{
    BTreePage page = getPage(pointer.pageIndex);
    if(pointer.cellIndex > 0) {
        pointer = {pointer.pageIndex, (BTreePage::Index)(pointer.cellIndex - 1)};
        return true;
    } else {
        if(page.prevSibling() == Page::kInvalidIndex) {
            pointer = {Page::kInvalidIndex, 0};
            return false;
        } else {
            pointer = {page.prevSibling(), (BTreePage::Index)(getPage(page.prevSibling()).numCells() - 1)};
            return true;
        }
    }
}

void BTree::print()
 {
    Page &page = mPageSet.page(mRootIndex);
    BTreePage(page, *mKeyDefinition, *mDataDefinition).print("");
}


BTreePage BTree::findLeaf(Key key, SearchComparison comparison, SearchPosition position)
{
    Page::Index index = mRootIndex;
    while(true) {
        BTreePage page = getPage(index);
        if(page.type() == BTreePage::Type::Leaf) {
            break;
        }
        
        index = page.indirectLookup(key, comparison, position);
    }

    return getPage(index);
}

BTreePage BTree::getPage(Page::Index index)
{
    return BTreePage(mPageSet.page(index), *mKeyDefinition, *mDataDefinition);
}

int BTree::keyCompare(Key a, Key b)
{
    return mKeyDefinition->compare(a, b);
}
