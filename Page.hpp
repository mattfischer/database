#ifndef PAGE_HPP
#define PAGE_HPP

#include <vector>

class PageSet;
class Page {
public:
    typedef size_t Index;
    static const Index kInvalidIndex = SIZE_MAX;

    Page(PageSet &pageSet, size_t size, Index index);

    PageSet &pageSet() const;
    size_t size() const;
    Index index() const;

    uint8_t *data(size_t offset = 0);
    const uint8_t *data(size_t offset = 0) const;

private:
    PageSet &mPageSet;
    std::vector<uint8_t> mData;
    Index mIndex;
};

#endif