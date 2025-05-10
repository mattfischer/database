#include "PageSet.hpp"

static const size_t kPageSize = 4096;

Page &PageSet::page(Page::Index index)
{ 
    return mPages[index];
}

Page &PageSet::addPage()
{
    mPages.emplace_back(kPageSize, mPages.size());

    return mPages.back();
}