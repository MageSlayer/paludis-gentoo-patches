/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_repository_filter_model.hh"
#include "main_window.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesRepositoryFilterModel>
    {
        MainWindow * const main_window;
        PackagesRepositoryFilterModel::Columns columns;

        Implementation(MainWindow * const m) :
            main_window(m)
        {
        }
    };
}

PackagesRepositoryFilterModel::PackagesRepositoryFilterModel(MainWindow * const m) :
    PrivateImplementationPattern<PackagesRepositoryFilterModel>(new Implementation<PackagesRepositoryFilterModel>(m)),
    Gtk::TreeStore(_imp->columns)
{
}

PackagesRepositoryFilterModel::~PackagesRepositoryFilterModel()
{
}

void
PackagesRepositoryFilterModel::populate()
{
    iterator r;

    r = append();
    (*r)[_imp->columns.col_text] = "All repositories";
    (*r)[_imp->columns.col_sensitive] = true;
    (*r)[_imp->columns.col_generator] = std::tr1::shared_ptr<Generator>(new generator::All());

    r = append();
    (*r)[_imp->columns.col_text] = "Installable repositories";
    (*r)[_imp->columns.col_sensitive] = true;
    (*r)[_imp->columns.col_generator] = std::tr1::shared_ptr<Generator>(new generator::SomeIDsMightSupportAction<InstallAction>());

    r = append();
    (*r)[_imp->columns.col_text] = "Installed repositories";
    (*r)[_imp->columns.col_sensitive] = true;
    (*r)[_imp->columns.col_generator] = std::tr1::shared_ptr<Generator>(new generator::SomeIDsMightSupportAction<InstalledAction>());

    _imp->main_window->paludis_thread_action(
            sigc::mem_fun(this, &PackagesRepositoryFilterModel::populate_in_paludis_thread), "Populating repository filter model");
}

void
PackagesRepositoryFilterModel::populate_in_paludis_thread()
{
    std::tr1::shared_ptr<RepositoryNameSequence> columns(new RepositoryNameSequence);

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator>
            r(indirect_iterator(_imp->main_window->environment()->package_database()->begin_repositories())),
            r_end(indirect_iterator(_imp->main_window->environment()->package_database()->end_repositories())) ;
            r != r_end ; ++r)
        columns->push_back(r->name());

    _imp->main_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &PackagesRepositoryFilterModel::populate_in_gui_thread), columns));
}

void
PackagesRepositoryFilterModel::populate_in_gui_thread(std::tr1::shared_ptr<const RepositoryNameSequence> names)
{
    iterator repositories(append());
    (*repositories)[_imp->columns.col_text] = "Specific repository";
    (*repositories)[_imp->columns.col_sensitive] = false;

    for (RepositoryNameSequence::ConstIterator n(names->begin()), n_end(names->end()) ;
            n != n_end ; ++n)
    {
        iterator r(append(repositories->children()));
        (*r)[_imp->columns.col_text] = stringify(*n);
        (*r)[_imp->columns.col_sensitive] = true;
        (*r)[_imp->columns.col_generator] = std::tr1::shared_ptr<Generator>(new generator::Repository(*n));
    }
}


PackagesRepositoryFilterModel::Columns::Columns()
{
    add(col_sensitive);
    add(col_text);
    add(col_generator);
}

PackagesRepositoryFilterModel::Columns::~Columns()
{
}

PackagesRepositoryFilterModel::Columns &
PackagesRepositoryFilterModel::columns()
{
    return _imp->columns;
}


