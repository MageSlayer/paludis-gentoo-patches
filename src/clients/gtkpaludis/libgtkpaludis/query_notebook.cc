/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "query_notebook.hh"
#include "query_notebook_page.hh"
#include "versions_page.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <map>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<QueryNotebook>
    {
        QueryWindow * const main_window;
        VersionsPage versions_page;

        std::map<int, QueryNotebookPage *> pages_by_index;

        sigc::connection signal_switch_page_connection;

        Implementation(QueryWindow * const m) :
            main_window(m),
            versions_page(m)
        {
        }
    };
}

QueryNotebook::QueryNotebook(QueryWindow * const m) :
    Gtk::Notebook(),
    PrivateImplementationPattern<QueryNotebook>(new Implementation<QueryNotebook>(m))
{
    _imp->pages_by_index.insert(std::make_pair(
                append_page(_imp->versions_page, "Versions"),
                &_imp->versions_page));

    _imp->signal_switch_page_connection = signal_switch_page().connect(
            sigc::mem_fun(*this, &QueryNotebook::handle_switch_page));
}

QueryNotebook::~QueryNotebook()
{
    _imp->signal_switch_page_connection.disconnect();
}

void
QueryNotebook::handle_switch_page(GtkNotebookPage *, guint page)
{
    std::map<int, QueryNotebookPage *>::const_iterator f(_imp->pages_by_index.find(page));
    if (_imp->pages_by_index.end() != f)
        f->second->populate_once();
}

