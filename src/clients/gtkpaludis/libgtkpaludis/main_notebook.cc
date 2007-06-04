/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "main_notebook.hh"
#include "main_notebook_page.hh"
#include "repositories_page.hh"
#include "packages_page.hh"
#include "messages_page.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <map>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<MainNotebook>
    {
        MainWindow * const main_window;

        PackagesPage packages_page;
        RepositoriesPage repositories_page;

        MessagesPage messages_page;
        bool messages_starred;

        std::map<int, MainNotebookPage *> pages_by_index;

        sigc::connection signal_switch_page_connection;

        Implementation(MainWindow * const m, MainNotebook * const n) :
            main_window(m),
            packages_page(m),
            repositories_page(m),
            messages_page(m, n),
            messages_starred(false)
        {
        }
    };
}

MainNotebook::MainNotebook(MainWindow * const m) :
    Gtk::Notebook(),
    PrivateImplementationPattern<MainNotebook>(new Implementation<MainNotebook>(m, this))
{
    _imp->pages_by_index.insert(std::make_pair(
                append_page(_imp->packages_page, "Packages"),
                &_imp->packages_page));

    _imp->pages_by_index.insert(std::make_pair(
                append_page(_imp->repositories_page, "Repositories"),
                &_imp->repositories_page));

    _imp->pages_by_index.insert(std::make_pair(
                append_page(_imp->messages_page, "Messages"),
                &_imp->messages_page));

    _imp->signal_switch_page_connection = signal_switch_page().connect(
            sigc::mem_fun(*this, &MainNotebook::handle_switch_page));
}

MainNotebook::~MainNotebook()
{
    _imp->signal_switch_page_connection.disconnect();
}

void
MainNotebook::handle_switch_page(GtkNotebookPage *, guint page)
{
    std::map<int, MainNotebookPage *>::const_iterator f(_imp->pages_by_index.find(page));
    if (_imp->pages_by_index.end() != f)
    {
        f->second->populate_once();
        if (f->second == &_imp->messages_page)
            unmark_messages_page();
    }
}

void
MainNotebook::mark_messages_page()
{
    if (! _imp->messages_starred)
    {
        _imp->messages_starred = true;
        set_tab_label_text(_imp->messages_page, "Messages *");
    }
}

void
MainNotebook::unmark_messages_page()
{
    if (_imp->messages_starred)
    {
        _imp->messages_starred = false;
        set_tab_label_text(_imp->messages_page, "Messages");
    }
}

