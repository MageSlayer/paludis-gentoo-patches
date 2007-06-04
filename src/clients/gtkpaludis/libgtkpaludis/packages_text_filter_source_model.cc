/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_text_filter_source_model.hh"
#include "main_window.hh"
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/iterator.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/query.hh>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesTextFilterSourceModel>
    {
        MainWindow * const main_window;
        PackagesTextFilterSourceModel::Columns columns;

        Implementation(MainWindow * const m) :
            main_window(m)
        {
        }
    };
}

PackagesTextFilterSourceModel::PackagesTextFilterSourceModel(MainWindow * const m) :
    PrivateImplementationPattern<PackagesTextFilterSourceModel>(new Implementation<PackagesTextFilterSourceModel>(m)),
    Gtk::TreeStore(_imp->columns)
{
}

PackagesTextFilterSourceModel::~PackagesTextFilterSourceModel()
{
}

void
PackagesTextFilterSourceModel::populate()
{
    iterator r;

    r = append();
    (*r)[_imp->columns.col_text] = "Name contains:";
    (*r)[_imp->columns.col_filter] = ptfso_name;

    r = append();
    (*r)[_imp->columns.col_text] = "Description contains:";
    (*r)[_imp->columns.col_filter] = ptfso_description;
}


PackagesTextFilterSourceModel::Columns::Columns()
{
    add(col_text);
    add(col_filter);
}

PackagesTextFilterSourceModel::Columns::~Columns()
{
}

PackagesTextFilterSourceModel::Columns &
PackagesTextFilterSourceModel::columns()
{
    return _imp->columns;
}

