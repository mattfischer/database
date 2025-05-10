#include "Page.hpp"

Page::Page(size_t size, Index index)
 : mData(size)
{
    mIndex = index;
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
