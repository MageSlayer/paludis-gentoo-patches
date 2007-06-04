/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "query_notebook_page.hh"

using namespace gtkpaludis;

QueryNotebookPage::QueryNotebookPage() :
    _populated(false)
{
}

QueryNotebookPage::~QueryNotebookPage()
{
}

void
QueryNotebookPage::populate_once()
{
    if (_populated)
        return;

    populate();
    _populated = true;
}


