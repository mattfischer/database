#include <iostream>

#include "PageSet.hpp"
#include "BTree.hpp"

int main(int argc, char *argv[])
{
    PageSet pageSet;
    BTree tree(pageSet);

    return 0;
}