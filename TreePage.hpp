#ifndef TREEPAGE_HPP
#define TREEPAGE_HPP

#include "PageSet.hpp"

#include <string>

class TreePage {
public:
    typedef uint16_t Index;
    typedef uint16_t Size;

    enum Type : uint8_t {
        Leaf,
        Indirect
    };

    class KeyDefinition {
    public:
        virtual ~KeyDefinition() = default;

        virtual Size fixedSize() = 0;
        virtual int compare(void *a, void *b) = 0;
        virtual void print(void *key) = 0;
    };

    struct KeyValue {
        std::vector<uint8_t> data;
    };

    TreePage(Page &page, KeyDefinition &keyDefinition);

    void initialize(Type type);

    Page &page();
    Page::Index pageIndex();

    Type type();

    Page::Index parent();
    void setParent(Page::Index parent);

    Size freeSpace();

    Index numCells();

    void *cellKey(Index index);
    Size cellKeySize(Index index);
    Size cellTotalKeySize(Index index);

    void *cellData(Index index);
    Size cellDataSize(Index index);
    Size cellTotalDataSize(Index index);

    void setCellKey(Index index, void *key, size_t keySize);

    void *insertCell(void *key, Size keySize, Size dataSize, Index index);

    void removeCell(Index index);
    void removeCells(Index begin, Index end);

    TreePage split(Index index);

    bool isDeficient();
    bool canSupplyItem(Index index);

    void *leafLookup(void *key);
    bool leafCanAdd(size_t keySize, size_t dataSize);
    void *leafAdd(void *key, size_t keySize, size_t dataSize);
    void leafRemove(void *key);

    bool indirectCanAdd(size_t keySize);
    void indirectAdd(void *key, size_t keySize, TreePage &childPage);
    Page::Index indirectPageIndex(Index index);
    Page::Index indirectLookup(void *key);
    KeyValue indirectRectifyDeficientChild(TreePage &childPage, void *removedKey);
    void indirectRotateRight(TreePage &leftChild, TreePage &rightChild, Index index);
    void indirectRotateLeft(TreePage &leftChild, TreePage &rightChild, Index index);
    void indirectMergeChildren(TreePage &leftChild, TreePage &rightChild, Index index);
    void indirectPushHead(void *oldHeadKey, Size oldHeadKeySize, TreePage &childPage);
    void indirectPushTail(void *key, Size keySize, TreePage &childPage);
    void indirectPopHead();
    void indirectPopTail();

    template <typename F> void print(const std::string &prefix, F printCell);

private:
    struct Header {
        Type type;
        uint16_t numCells;
        uint16_t dataStart;
        uint16_t freeSpace;
        Page::Index parent;
    };

    Header &header();
    uint16_t *offsetsArray();

    void *cell(Index index);
    Size cellSize(Index index);
    uint16_t cellOffset(Index index);
    uint16_t cellKeyOffset(Index index); 
    uint16_t cellDataOffset(Index index);

    uint16_t allocateCell(void *key, Size keySize, Size dataSize);
    bool canAllocateCell(Size keySize, Size dataSize);

    Index search(void *key);

    void defragPage();

    int keyCompare(void *a, void *b);
    TreePage getPage(Page::Index index);

    PageSet &pageSet();

    Page &mPage;
    KeyDefinition &mKeyDefinition;
};

template <typename F> void TreePage::print(const std::string &prefix, F printCell)
{
    switch(type()) {
        case TreePage::Type::Leaf:
            std::cout << prefix << "# Leaf page " << pageIndex();
            if(parent() != Page::kInvalidIndex) {
                std::cout << " (parent " << parent() << ")";
            }
            std::cout << std::endl;

            for(Index i=0; i<numCells(); i++) {
                std::cout << prefix;
                mKeyDefinition.print(cellKey(i));
                std::cout << ": ";
                printCell(cellData(i));
                std::cout << std::endl;
            }
            break;
        case TreePage::Type::Indirect:
            std::cout << prefix << "# Indirect page " << pageIndex();
            if(parent() != Page::kInvalidIndex) {
                std::cout << " (parent " << parent() << ")";
            }
            std::cout << std::endl;

            for(Index i=0; i<numCells(); i++) {
                std::cout << prefix;
                if(i == 0) {
                    std::cout << "_";
                } else {
                    mKeyDefinition.print(cellKey(i));
                }
                std::cout << ": " << std::endl;

                Page::Index index = indirectPageIndex(i);
                TreePage(pageSet().page(index), mKeyDefinition).print(prefix + "  ", printCell);
            }
            break;
    }
}

#endif