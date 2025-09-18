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

    srand(12345);
    for(int i=0; i<32; i++) {
        uint32_t key = rand() % 1000;
        tree.add(key, 0x10);
    }
    tree.print();

    srand(12345);
    for(int i=0; i<32; i++) {
        uint32_t key = rand() % 1000;
        tree.remove(key);
        tree.print();
        std::cout << "----------" << std::endl;
    }

    return 0;
}