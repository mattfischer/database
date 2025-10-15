#ifndef ROWITERATOR_HPP
#define ROWITERATOR_HPP

#include "Record.hpp"
#include "Value.hpp"

class RowIterator {
public:
    virtual ~RowIterator() = default;

    virtual RecordSchema schema() = 0;

    virtual void start() = 0;
    virtual bool valid() = 0;

    virtual void prev() = 0;
    virtual void next() = 0;

    virtual Value getField(unsigned int index) = 0;
};

#endif