/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "main_notebook_page.hh"

using namespace gtkpaludis;

MainNotebookPage::MainNotebookPage() :
    _populated(false)
{
}

MainNotebookPage::~MainNotebookPage()
{
}

void
MainNotebookPage::populate_once()
{
    if (_populated)
        return;

    populate();
    _populated = true;
}

