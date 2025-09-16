#ifndef LEAFPAGE_HPP
#define LEAFPAGE_HPP

#include "TreePage.hpp"

class LeafPage : public TreePage {
public:
    LeafPage(Page &page);

    void initialize();

    void *lookup(RowId rowId);
    bool canAdd(size_t size);
    void *add(RowId rowId, size_t size);
    void remove(RowId rowId);

    LeafPage split(PageSet &pageSet);

    void print();
};

#endif