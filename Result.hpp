#ifndef RESULT_HPP
#define RESULT_HPP

#include "BTree.hpp"

class Table;
class Result {
public:
    Result(Table &table, BTree::Pointer start);

    bool valid();

    void prev();
    void next();

    void *data();

private:
    Table &mTable;
    BTree::Pointer mPointer;
};

#endif