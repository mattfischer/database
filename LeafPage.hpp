#ifndef LEAFPAGE_HPP
#define LEAFPAGE_HPP

#include "TreePage.hpp"

#include <tuple>

class LeafPage : public TreePage {
public:
    LeafPage(Page &page);

    void initialize();

    void *lookup(RowId rowId);
    bool canAdd(size_t size);
    void *add(RowId rowId, size_t size);
    void remove(RowId rowId);

    std::tuple<LeafPage, RowId> split();

    void print(const std::string &prefix);
};

#endif