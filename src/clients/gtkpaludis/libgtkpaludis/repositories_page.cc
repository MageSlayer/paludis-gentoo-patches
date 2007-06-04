/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "repositories_page.hh"
#include "repositories_list.hh"
#include "repository_info.hh"
#include "repository_buttons.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <gtkmm/scrolledwindow.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<RepositoriesPage>
    {
        MainWindow * const main_window;

        Gtk::ScrolledWindow repositories_list_scroll;
        RepositoriesList repositories_list;

        Gtk::ScrolledWindow repository_info_scroll;
        RepositoryInfo repository_info;

        RepositoryButtons repository_buttons;

        Implementation(MainWindow * const m, RepositoriesPage * const p) :
            main_window(m),
            repositories_list(m, p),
            repository_info(m),
            repository_buttons(m)
        {
        }
    };
}

RepositoriesPage::RepositoriesPage(MainWindow * const m) :
    Gtk::Table(2, 2),
    MainNotebookPage(),
    PrivateImplementationPattern<RepositoriesPage>(new Implementation<RepositoriesPage>(m, this))
{
    _imp->repositories_list_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _imp->repositories_list_scroll.add(_imp->repositories_list);
    attach(_imp->repositories_list_scroll, 0, 1, 0, 2, Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL, 4, 4);

    _imp->repository_info_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _imp->repository_info_scroll.add(_imp->repository_info);
    attach(_imp->repository_info_scroll, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 4, 4);

    attach(_imp->repository_buttons, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL, 4, 4);
}

RepositoriesPage::~RepositoriesPage()
{
}

void
RepositoriesPage::populate()
{
    _imp->repositories_list.populate();
}

void
RepositoriesPage::set_repository(const RepositoryName & name)
{
    _imp->repository_info.set_repository(name);
    _imp->repository_buttons.set_repository(name);
}

