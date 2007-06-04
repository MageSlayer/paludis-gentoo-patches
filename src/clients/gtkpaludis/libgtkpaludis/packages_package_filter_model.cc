/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_package_filter_model.hh"
#include "main_window.hh"
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/iterator.hh>
#include <paludis/environment.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/package_database.hh>
#include <paludis/query.hh>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesPackageFilterModel>
    {
        MainWindow * const main_window;
        PackagesPackageFilterModel::Columns columns;

        Implementation(MainWindow * const m) :
            main_window(m)
        {
        }
    };
}

PackagesPackageFilterModel::PackagesPackageFilterModel(MainWindow * const m) :
    PrivateImplementationPattern<PackagesPackageFilterModel>(new Implementation<PackagesPackageFilterModel>(m)),
    Gtk::TreeStore(_imp->columns)
{
}

PackagesPackageFilterModel::~PackagesPackageFilterModel()
{
}

void
PackagesPackageFilterModel::populate()
{
    iterator r;

    r = append();
    (*r)[_imp->columns.col_text] = "All packages";
    (*r)[_imp->columns.col_filter] = ppfo_all_packages;

    r = append();
    (*r)[_imp->columns.col_text] = "Visible packages";
    (*r)[_imp->columns.col_filter] = ppfo_visible_packages;

    r = append();
    (*r)[_imp->columns.col_text] = "Installed packages";
    (*r)[_imp->columns.col_filter] = ppfo_installed_packages;

    r = append();
    (*r)[_imp->columns.col_text] = "Upgradable packages";
    (*r)[_imp->columns.col_filter] = ppfo_upgradable_packages;
}


PackagesPackageFilterModel::Columns::Columns()
{
    add(col_text);
    add(col_filter);
}

PackagesPackageFilterModel::Columns::~Columns()
{
}

PackagesPackageFilterModel::Columns &
PackagesPackageFilterModel::columns()
{
    return _imp->columns;
}

