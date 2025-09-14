#ifndef PAGE_HPP
#define PAGE_HPP

#include <vector>

class Page {
public:
    typedef size_t Index;
    static const Index kInvalidIndex = SIZE_MAX;

    Page(size_t size, Index index);

    size_t size() const;
    Index index() const;

    uint8_t *data(size_t offset = 0);
    const uint8_t *data(size_t offset = 0) const;

private:
    std::vector<uint8_t> mData;
    Index mIndex;
};

#endif