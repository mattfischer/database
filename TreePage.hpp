#ifndef TREEPAGE_HPP
#define TREEPAGE_HPP

#include "PageSet.hpp"

#include <string>

class TreePage {
public:
    typedef uint16_t Index;
    static const Index kInvalidIndex = UINT16_MAX;

    typedef uint16_t Size;

    enum Type : uint8_t {
        Leaf,
        Indirect
    };

    struct Key {
        Key() = default;
        Key(void *d, Size s) : data(d), size(s) {}

        operator bool() { return data; }

        void *data = nullptr;
        Size size = 0;
    };

    class KeyDefinition {
    public:
        virtual ~KeyDefinition() = default;

        virtual Size fixedSize() = 0;
        virtual int compare(Key a, Key b) = 0;
        virtual void print(Key key) = 0;
    };

    TreePage(Page &page, KeyDefinition &keyDefinition);

    void initialize(Type type);

    Page &page();
    Page::Index pageIndex();

    Type type();

    Page::Index parent();
    void setParent(Page::Index parent);

    Page::Index prevSibling();
    void setPrevSibling(Page::Index prevSibling);

    Page::Index nextSibling();
    void setNextSibling(Page::Index prevSibling);

    Index numCells();

    Key cellKey(Index index);
    void setCellKey(Index index, Key key);

    void *cellData(Index index);
    Size cellDataSize(Index index);

    Index insertCell(Key key, Size dataSize, Index index);

    void removeCell(Index index);

    TreePage split(Index index);

    bool isDeficient();
    bool canSupplyItem(Index index);

    Index leafLookup(Key key);
    bool leafCanAdd(size_t keySize, size_t dataSize);
    Index leafAdd(Key key, size_t dataSize);
    void leafRemove(Index index);

    bool indirectCanAdd(size_t keySize);
    void indirectAdd(Key key, TreePage &childPage);
    Page::Index indirectPageIndex(Index index);
    Page::Index indirectLookup(Key key);
    void indirectRectifyDeficientChild(TreePage &childPage);
    void indirectPushHead(Key oldHeadKey, TreePage &childPage);
    void indirectPushTail(Key key, TreePage &childPage);
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
        Page::Index prevSibling;
        Page::Index nextSibling;
    };

    Header &header();
    uint16_t *offsetsArray();

    void *cell(Index index);
    Size cellSize(Index index);
    uint16_t cellOffset(Index index);
    uint16_t cellKeyOffset(Index index); 
    uint16_t cellDataOffset(Index index);
    Size cellTotalKeySize(Index index);
    Size cellTotalDataSize(Index index);

    uint16_t allocateCell(Key key, Size dataSize);
    bool canAllocateCell(Size keySize, Size dataSize);

    void removeCells(Index begin, Index end);

    Index search(Key key);

    Size freeSpace();
    void defragPage();

    int keyCompare(Key a, Key b);
    TreePage getPage(Page::Index index);

    PageSet &pageSet();

    void indirectRotateRight(TreePage &leftChild, TreePage &rightChild, Index index);
    void indirectRotateLeft(TreePage &leftChild, TreePage &rightChild, Index index);
    void indirectMergeChildren(TreePage &leftChild, TreePage &rightChild, Index index);

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