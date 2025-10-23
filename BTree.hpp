#ifndef BTREE_HPP
#define BTREE_HPP

#include "PageSet.hpp"

#include "BTreePage.hpp"

#include <memory>
#include <iostream>

class BTree {
public:
    typedef BTreePage::Key Key;
    typedef BTreePage::KeyDefinition KeyDefinition;
    typedef BTreePage::DataDefinition DataDefinition;
    typedef BTreePage::SearchComparison SearchComparison;
    typedef BTreePage::SearchPosition SearchPosition;

    struct Pointer {
        Page::Index pageIndex;
        BTreePage::Index cellIndex;
    };

    struct KeyValue {
        KeyValue() = default;
        KeyValue(Key key) {
            data.resize(key.size);
            std::memcpy(data.data(), key.data, key.size);
        }
        KeyValue(size_t keySize) {
            data.resize(keySize);
        }

        operator Key() {
            return Key(data.data(), data.size());
        }

        std::vector<uint8_t> data;
    };

    BTree(PageSet &pageSet, Page::Index rootIndex, std::unique_ptr<KeyDefinition> keyDefinition, std::unique_ptr<DataDefinition> dataDefinition);

    void initialize();

    PageSet &pageSet();

    Pointer lookup(Key key, SearchComparison comparison, SearchPosition position);
    Pointer add(Key key, BTreePage::Size size);
    void remove(Pointer pointer);

    void *key(Pointer pointer);
    void *data(Pointer pointer);

    Pointer first();
    Pointer last();

    Pointer next(Pointer pointer);
    Pointer prev(Pointer pointer);

    void print();

private:
    PageSet &mPageSet;
    Page::Index mRootIndex;
    std::unique_ptr<KeyDefinition> mKeyDefinition;
    std::unique_ptr<DataDefinition> mDataDefinition;

    BTreePage findLeaf(Key key, SearchComparison comparison, SearchPosition position);
    BTreePage getPage(Page::Index index);
    int keyCompare(Key a, Key b);
};

#endif