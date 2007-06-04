/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "version_info.hh"
#include "version_info_model.hh"
#include "versions_page.hh"
#include "query_window.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<VersionInfo>
    {
        Glib::RefPtr<VersionInfoModel> model;
        QueryWindow * const query_window;
        VersionsPage * const versions_page;

        Implementation(QueryWindow * const m, VersionsPage * const p) :
            model(new VersionInfoModel(m, p)),
            query_window(m),
            versions_page(p)
        {
        }
    };
}

VersionInfo::VersionInfo(QueryWindow * const m, VersionsPage * const p) :
    Gtk::TreeView(),
    PrivateImplementationPattern<VersionInfo>(new Implementation<VersionInfo>(m, p))
{
    set_model(_imp->model);

    append_column("Key", _imp->model->columns().col_key);

    Gtk::CellRendererText renderer;
    int c(append_column("Value", renderer) - 1);
    get_column(c)->add_attribute(renderer, "markup", _imp->model->columns().col_value_markup.index());
}

VersionInfo::~VersionInfo()
{
}

void
VersionInfo::populate()
{
    _imp->model->populate();
    _imp->query_window->paludis_thread_action(
            sigc::mem_fun(this, &VersionInfo::populate_in_paludis_thread), "Populating version information");
}

void
VersionInfo::populate_in_paludis_thread()
{
    _imp->query_window->gui_thread_action(
            sigc::mem_fun(this, &VersionInfo::populate_in_gui_thread));
}

void
VersionInfo::populate_in_gui_thread()
{
    expand_all();
    columns_autosize();
}

