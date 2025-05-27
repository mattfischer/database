#include <iostream>

#include "PageSet.hpp"
#include "CellPage.hpp"
#include "BTree.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    PageSet pageSet;
    Page &page = pageSet.addPage();
    BTree tree(pageSet, page.index());
    tree.intialize();

    tree.add(5, 50);
    tree.add(2, 20);
    tree.add(8, 80);
    tree.add(7, 70);
    
    tree.print();

    return 0;
}