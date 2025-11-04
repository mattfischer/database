#ifndef ROWITERATOR_HPP
#define ROWITERATOR_HPP

#include "Record.hpp"
#include "Value.hpp"

class RowIterator {
public:
    virtual ~RowIterator() = default;

    virtual Record::Schema &schema() = 0;

    virtual void start() = 0;
    virtual bool valid() = 0;
    virtual void next() = 0;
    virtual bool remove() = 0;

    struct ModifyEntry {
        unsigned int field;
        Value value;
    };
    virtual bool modify(std::vector<ModifyEntry> entries) = 0;

    virtual Value getField(unsigned int index) = 0;
};

#endif