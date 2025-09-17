#ifndef INDIRECTPAGE_HPP
#define INDIRECTPAGE_HPP

#include "TreePage.hpp"

#include <tuple>

class IndirectPage : public TreePage
{
public:
    IndirectPage(Page &page);

    void initialize();

    bool canAdd();
    void add(RowId rowId, TreePage &childPage);

    Page::Index lookup(RowId rowId);
    std::tuple<IndirectPage, RowId> split();

    void print(const std::string &prefix);

    Page::Index cellPageIndex(CellPage::Index index);
};

#endif