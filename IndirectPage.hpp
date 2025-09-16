#ifndef INDIRECTPAGE_HPP
#define INDIRECTPAGE_HPP

#include "TreePage.hpp"

class IndirectPage : public TreePage
{
public:
    IndirectPage(Page &page);

    void initialize();

    bool canAdd();
    void add(TreePage &childPage);

    Page::Index lookup(RowId rowId);
    IndirectPage split();

    Page::Index cellPageIndex(CellPage::Index index);
};

#endif