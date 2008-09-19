/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "versions_list_model.hh"
#include "query_window.hh"
#include "versions_page.hh"
#include "markup.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <tr1/functional>
#include <list>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<VersionsListModel>
    {
        QueryWindow * const query_window;
        VersionsPage * const versions_page;
        VersionsListModel::Columns columns;

        Implementation(QueryWindow * const m, VersionsPage * const p) :
            query_window(m),
            versions_page(p)
        {
        }
    };
}

namespace
{
    struct PopulateDataItem
    {
        std::tr1::shared_ptr<const PackageID> id;
        std::string masks;
        bool prefer_default;

        PopulateDataItem(const std::tr1::shared_ptr<const PackageID> & d,
                const std::string & m, const bool p) :
            id(d),
            masks(m),
            prefer_default(p)
        {
        }
    };
}

namespace gtkpaludis
{
    struct VersionsListModel::PopulateData
    {
        std::list<PopulateDataItem> items;
    };
}


VersionsListModel::VersionsListModel(QueryWindow * const m, VersionsPage * const p) :
    PrivateImplementationPattern<VersionsListModel>(new Implementation<VersionsListModel>(m, p)),
    Gtk::TreeStore(_imp->columns)
{
}

VersionsListModel::~VersionsListModel()
{
}

VersionsListModel::Columns::Columns()
{
    add(col_version_string);
    add(col_repo_name);
    add(col_slot);
    add(col_masks_markup);
    add(col_id);
    add(col_prefer_default);
}

VersionsListModel::Columns::~Columns()
{
}

VersionsListModel::Columns &
VersionsListModel::columns()
{
    return _imp->columns;
}

void
VersionsListModel::populate()
{
    _imp->query_window->paludis_thread_action(
            sigc::mem_fun(this, &VersionsListModel::populate_in_paludis_thread),
            "Populating versions list model");
}

void
VersionsListModel::populate_in_paludis_thread()
{
    std::tr1::shared_ptr<PopulateData> data(new PopulateData);
    std::tr1::shared_ptr<const PackageIDSequence> c(
            (*_imp->query_window->environment())[selection::AllVersionsSorted(
                generator::Package(_imp->query_window->get_package_name()))]);

    for (PackageIDSequence::ReverseConstIterator p(c->rbegin()), p_end(c->rend()) ;
            p != p_end ; ++p)
    {
        bool prefer_default(true);
        std::string mr_string;

        for (PackageID::MasksConstIterator m((*p)->begin_masks()), m_end((*p)->end_masks()) ;
                m != m_end ; ++m)
        {
            prefer_default = false;
            if (! mr_string.empty())
                mr_string.append(", ");

            mr_string.append((*m)->description());
        }

        data->items.push_back(PopulateDataItem(*p, mr_string, prefer_default));
    }

    _imp->query_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &VersionsListModel::populate_in_gui_thread), data));
}

void
VersionsListModel::populate_in_gui_thread(const std::tr1::shared_ptr<const VersionsListModel::PopulateData> & names)
{
    clear();

    for (std::list<PopulateDataItem>::const_iterator i(names->items.begin()), i_end(names->items.end()) ;
            i != i_end ; ++i)
    {
        iterator r(append());
        (*r)[_imp->columns.col_id] = i->id;
        (*r)[_imp->columns.col_repo_name] = stringify(i->id->repository()->name());
        (*r)[_imp->columns.col_version_string] = stringify(i->id->canonical_form(idcf_version));
        (*r)[_imp->columns.col_slot] = stringify(i->id->slot());
        (*r)[_imp->columns.col_masks_markup] = i->masks;
        (*r)[_imp->columns.col_prefer_default] = i->prefer_default;
    }
}


