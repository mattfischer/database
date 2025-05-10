#include "BTree.hpp"

BTree::BTree(PageSet &pageSet)
: mPageSet(pageSet)
{
    LeafPage leaf = addLeafPage();
    mRoot = leaf.page().index();
}

BTree::LeafPage BTree::addLeafPage()
{
    Page &page = mPageSet.addPage();
    LeafPage leafPage(page);
    return leafPage;
}

BTree::LeafPage::LeafPage(Page &page)
: mCellPage(page, sizeof(PageHeader), 0)
{
}

void BTree::LeafPage::initialize()
{
    mCellPage.initialize();
    header().type = PageHeader::Type::Leaf;
}

Page &BTree::LeafPage::page()
{
    return mCellPage.page();
}

BTree::PageHeader &BTree::LeafPage::header()
{
    return *reinterpret_cast<PageHeader*>(mCellPage.extraHeader());
}