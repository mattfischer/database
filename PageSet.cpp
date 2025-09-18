#include "PageSet.hpp"

static const size_t kPageSize = 128;

Page &PageSet::page(Page::Index index)
{ 
    return *mPages[index];
}

Page &PageSet::addPage()
{
    mPages.push_back(std::make_unique<Page>(*this, kPageSize, mPages.size()));

    return *mPages.back();
}

void PageSet::deletePage(Page &page)
{
}