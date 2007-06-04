/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "versions_list.hh"
#include "versions_list_model.hh"
#include "versions_page.hh"
#include "query_window.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<VersionsList>
    {
        Glib::RefPtr<VersionsListModel> model;
        QueryWindow * const query_window;
        VersionsPage * const versions_page;

        Implementation(QueryWindow * const m, VersionsPage * const p) :
            model(new VersionsListModel(m, p)),
            query_window(m),
            versions_page(p)
        {
        }
    };
}

VersionsList::VersionsList(QueryWindow * const m, VersionsPage * const p) :
    Gtk::TreeView(),
    PrivateImplementationPattern<VersionsList>(new Implementation<VersionsList>(m, p))
{
    set_model(_imp->model);

    append_column("Version", _imp->model->columns().col_version_string);
    append_column("Repository", _imp->model->columns().col_repo_name);
    append_column("Slot", _imp->model->columns().col_slot);

    Gtk::CellRendererText renderer;
    int c(append_column("Masks", renderer) - 1);
    get_column(c)->add_attribute(renderer, "markup", _imp->model->columns().col_masks_markup.index());

    signal_cursor_changed().connect(sigc::mem_fun(this, &VersionsList::handle_signal_cursor_changed));
}

VersionsList::~VersionsList()
{
}

void
VersionsList::handle_signal_cursor_changed()
{
    if (get_selection()->get_selected())
        _imp->versions_page->set_pde(paludis::tr1::shared_ptr<PackageDatabaseEntry>(new PackageDatabaseEntry(*
                        (*get_selection()->get_selected())[_imp->model->columns().col_pde].operator
                        paludis::tr1::shared_ptr<const PackageDatabaseEntry> ())));
    else
        _imp->versions_page->set_pde(paludis::tr1::shared_ptr<PackageDatabaseEntry>());
}

void
VersionsList::populate()
{
    _imp->model->populate();
    _imp->query_window->paludis_thread_action(
            sigc::mem_fun(this, &VersionsList::populate_in_paludis_thread), "Populating versions list");
}

void
VersionsList::populate_in_paludis_thread()
{
    _imp->query_window->gui_thread_action(
            sigc::mem_fun(this, &VersionsList::populate_in_gui_thread));
}

void
VersionsList::populate_in_gui_thread()
{
    expand_all();
    columns_autosize();

    if (! get_selection()->get_selected())
        if (! _imp->model->children().empty())
        {
            bool found(false);
            for (Gtk::TreeModel::iterator i(_imp->model->children().begin()), i_end(_imp->model->children().end()) ;
                    i != i_end && ! found; ++i)
                if ((*i)[_imp->model->columns().col_prefer_default])
                {
                    set_cursor(_imp->model->get_path(*i));
                    found = true;
                }

            if (! found)
                set_cursor(_imp->model->get_path(_imp->model->children().begin()));
        }
}

