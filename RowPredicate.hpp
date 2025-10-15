#ifndef ROWPREDICATE_HPP
#define ROWPREDICATE_HPP

#include "RowIterator.hpp"

class RowPredicate {
public:
    virtual ~RowPredicate() = default;

    virtual bool evaluate(RowIterator &iterator) = 0;
};

#endif