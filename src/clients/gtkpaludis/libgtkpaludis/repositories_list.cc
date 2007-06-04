/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "repositories_list.hh"
#include "repositories_list_model.hh"
#include "repositories_page.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<RepositoriesList>
    {
        Glib::RefPtr<RepositoriesListModel> model;
        MainWindow * const main_window;
        RepositoriesPage * const repositories_page;

        Implementation(MainWindow * const m, RepositoriesPage * const p) :
            model(new RepositoriesListModel(m)),
            main_window(m),
            repositories_page(p)
        {
        }
    };
}

RepositoriesList::RepositoriesList(MainWindow * const m, RepositoriesPage * const p) :
    Gtk::TreeView(),
    PrivateImplementationPattern<RepositoriesList>(new Implementation<RepositoriesList>(m, p))
{
    set_model(_imp->model);

    append_column("Repository", _imp->model->columns().col_repo_name);

    signal_cursor_changed().connect(sigc::mem_fun(this, &RepositoriesList::handle_signal_cursor_changed));
}

RepositoriesList::~RepositoriesList()
{
}

void
RepositoriesList::populate()
{
    _imp->model->populate();
}

void
RepositoriesList::handle_signal_cursor_changed()
{
    if (get_selection()->get_selected())
        _imp->repositories_page->set_repository(RepositoryName(
                    static_cast<Glib::ustring>((*get_selection()->get_selected())[_imp->model->columns().col_repo_name]).raw()));
}

