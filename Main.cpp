#include <iostream>

#include "PageSet.hpp"
#include "Page.hpp"
#include "CellPage.hpp"

int main(int argc, char *argv[])
{
    PageSet pageSet;

    Page &page = pageSet.addPage();
    CellPage cellPage(page);

    return 0;
}