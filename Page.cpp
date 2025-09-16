#include "Page.hpp"

Page::Page(PageSet &pageSet, size_t size, Index index)
 : mPageSet(pageSet), mData(size)
{
    mIndex = index;
}

PageSet &Page::pageSet() const
{
    return mPageSet;
}

size_t Page::size() const
{
    return mData.size();
}
Page::Index Page::index() const
{
    return mIndex;
}

uint8_t *Page::data(size_t offset)
{
    return mData.data() + offset;
}

const uint8_t *Page::data(size_t offset) const
{
    return mData.data() + offset;
}
