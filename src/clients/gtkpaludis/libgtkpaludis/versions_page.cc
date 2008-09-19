/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "versions_page.hh"
#include "versions_list.hh"
#include "version_info.hh"
#include <gtkmm/scrolledwindow.h>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<VersionsPage>
    {
        QueryWindow * const query_window;

        Gtk::ScrolledWindow versions_list_scroll;
        VersionsList versions_list;

        Gtk::ScrolledWindow version_info_scroll;
        VersionInfo version_info;

        std::tr1::shared_ptr<const PackageID> id;

        Implementation(QueryWindow * const m, VersionsPage * const p) :
            query_window(m),
            versions_list(m, p),
            version_info(m, p)
        {
        }
    };
}

VersionsPage::VersionsPage(QueryWindow * const m) :
    Gtk::Table(2, 1),
    QueryNotebookPage(),
    PrivateImplementationPattern<VersionsPage>(new Implementation<VersionsPage>(m, this))
{
    _imp->versions_list_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
    _imp->versions_list_scroll.add(_imp->versions_list);
    attach(_imp->versions_list_scroll, 0, 1, 0, 1, Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 4, 4);

    _imp->version_info_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _imp->version_info_scroll.add(_imp->version_info);
    attach(_imp->version_info_scroll, 0, 1, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 4, 4);
}

VersionsPage::~VersionsPage()
{
}

void
VersionsPage::populate()
{
    _imp->versions_list.populate();
}

void
VersionsPage::set_id(const std::tr1::shared_ptr<const PackageID> & c)
{
    _imp->id = c;
    _imp->version_info.populate();
}

std::tr1::shared_ptr<const PackageID>
VersionsPage::get_id() const
{
    return _imp->id;
}

