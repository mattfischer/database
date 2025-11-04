#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "BTreePage.hpp"

#include <memory>
#include <iostream>
#include <span>

class BTree {
public:
    typedef BTreePage::Key Key;
    typedef BTreePage::KeyDefinition KeyDefinition;
    typedef BTreePage::DataDefinition DataDefinition;
    typedef BTreePage::SearchComparison SearchComparison;
    typedef BTreePage::SearchPosition SearchPosition;
    typedef BTreePage::Pointer Pointer;
    typedef BTreePage::KeyComparator KeyComparator;

    BTree(PageSet &pageSet, Page::Index rootIndex, std::unique_ptr<KeyDefinition> keyDefinition, std::unique_ptr<DataDefinition> dataDefinition);

    void initialize();

    PageSet &pageSet();

    Pointer lookup(Key key, KeyComparator &comparator, SearchComparison comparison, SearchPosition position);
    Pointer lookup(Key key, SearchComparison comparison, SearchPosition position);
    Pointer add(Key key, BTreePage::Size size);
    bool resize(Pointer pointer, BTreePage::Size size);
    void remove(Pointer pointer, std::span<Pointer*> trackPointers = std::span<Pointer*>());

    void *key(Pointer pointer);
    void *data(Pointer pointer);

    Pointer first();
    Pointer last();

    bool moveNext(Pointer &pointer);
    bool movePrev(Pointer &pointer);

    void print();

private:
    PageSet &mPageSet;
    Page::Index mRootIndex;
    std::unique_ptr<KeyDefinition> mKeyDefinition;
    std::unique_ptr<DataDefinition> mDataDefinition;

    BTreePage findLeaf(Key key, KeyComparator &comparator, SearchComparison comparison, SearchPosition position);
    BTreePage findLeaf(Key key, SearchComparison comparison, SearchPosition position);

    BTreePage getPage(Page::Index index);
    int keyCompare(Key a, Key b);
};

#endif