/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "repositories_list_model.hh"
#include "main_window.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<RepositoriesListModel>
    {
        MainWindow * const main_window;
        RepositoriesListModel::Columns columns;

        Implementation(MainWindow * const m) :
            main_window(m)
        {
        }
    };
}

RepositoriesListModel::RepositoriesListModel(MainWindow * const m) :
    PrivateImplementationPattern<RepositoriesListModel>(new Implementation<RepositoriesListModel>(m)),
    Gtk::ListStore(_imp->columns)
{
}

RepositoriesListModel::~RepositoriesListModel()
{
}

void
RepositoriesListModel::populate()
{
    _imp->main_window->paludis_thread_action(
            sigc::mem_fun(this, &RepositoriesListModel::populate_in_paludis_thread), "Populating repositories list model");
}

void
RepositoriesListModel::populate_in_paludis_thread()
{
    tr1::shared_ptr<RepositoryNameSequence> columns(new RepositoryNameSequence);

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator>
            r(indirect_iterator(_imp->main_window->environment()->package_database()->begin_repositories())),
            r_end(indirect_iterator(_imp->main_window->environment()->package_database()->end_repositories())) ;
            r != r_end ; ++r)
        columns->push_back(r->name());

    _imp->main_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &RepositoriesListModel::populate_in_gui_thread), columns));
}

void
RepositoriesListModel::populate_in_gui_thread(tr1::shared_ptr<const RepositoryNameSequence> names)
{
    clear();
    for (RepositoryNameSequence::ConstIterator n(names->begin()), n_end(names->end()) ;
            n != n_end ; ++n)
        (*append())[_imp->columns.col_repo_name] = stringify(*n);
}


RepositoriesListModel::Columns::Columns()
{
    add(col_repo_name);
}

RepositoriesListModel::Columns::~Columns()
{
}

RepositoriesListModel::Columns &
RepositoriesListModel::columns()
{
    return _imp->columns;
}

