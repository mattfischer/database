#ifndef PAGESET_HPP
#define PAGESET_HPP

#include "Page.hpp"

#include <vector>
#include <memory>

class PageSet {
public:
    Page &page(Page::Index index);

    Page &addPage();

private:
    std::vector<std::unique_ptr<Page>> mPages;
};

#endif