#ifndef PAGESET_HPP
#define PAGESET_HPP

#include "Page.hpp"

#include <vector>

class PageSet {
public:
    Page &page(Page::Index index);

    Page &addPage();

private:
    std::vector<Page> mPages;
};

#endif