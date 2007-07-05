/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "versions_list_model.hh"
#include "query_window.hh"
#include "versions_page.hh"
#include "markup.hh"
#include <paludis/util/iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/sequence.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/query.hh>
#include <paludis/package_id.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
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
        tr1::shared_ptr<const PackageID> id;
        std::string masks;
        bool prefer_default;

        PopulateDataItem(const tr1::shared_ptr<const PackageID> & d,
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
    paludis::tr1::shared_ptr<PopulateData> data(new PopulateData);
    paludis::tr1::shared_ptr<const PackageIDSequence> c(
            _imp->query_window->environment()->package_database()->query(
                query::Package(_imp->query_window->get_package_name()),
                qo_order_by_version));

    for (PackageIDSequence::ReverseIterator p(c->rbegin()), p_end(c->rend()) ;
            p != p_end ; ++p)
    {
        bool prefer_default(true);

        MaskReasons mask_reasons(_imp->query_window->environment()->mask_reasons(**p));
        std::string mr_string;

        for (MaskReason m(MaskReason(0)) ; m < last_mr ;
                m = MaskReason(static_cast<int>(m) + 1))
        {
            if (! mask_reasons[m])
                continue;

            prefer_default = false;
            if (! mr_string.empty())
                mr_string.append(", ");

            switch (m)
            {
                case mr_keyword:
                    mr_string.append("keyword");
                    break;
                case mr_user_mask:
                    mr_string.append("user mask");
                    break;
                case mr_profile_mask:
                    mr_string.append("profile mask");
                    break;
                case mr_repository_mask:
                    mr_string.append("repository mask");
                    break;
                case mr_eapi:
                    mr_string.append("EAPI");
                    break;
                case mr_license:
                    mr_string.append("licence");
                    break;
                case mr_by_association:
                    mr_string.append("by association");
                    break;
                case mr_chost:
                    mr_string.append("wrong CHOST");
                    break;
                case mr_breaks_portage:
                    mr_string.append("breaks Portage");
                    break;

                case last_mr:
                    break;
            }
        }

        data->items.push_back(PopulateDataItem(*p, mr_string, prefer_default));
    }

    _imp->query_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &VersionsListModel::populate_in_gui_thread), data));
}

void
VersionsListModel::populate_in_gui_thread(paludis::tr1::shared_ptr<const VersionsListModel::PopulateData> names)
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


