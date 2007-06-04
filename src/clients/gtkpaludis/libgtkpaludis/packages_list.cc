/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_list.hh"
#include "packages_list_model.hh"
#include "packages_list_filtered_model.hh"
#include "packages_page.hh"
#include "main_window.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesList>
    {
        Glib::RefPtr<PackagesListModel> real_model;
        Glib::RefPtr<PackagesListFilteredModel> filtered_model;
        MainWindow * const main_window;
        PackagesPage * const repositories_page;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            real_model(new PackagesListModel(m, p)),
            filtered_model(new PackagesListFilteredModel(m, p, real_model)),
            main_window(m),
            repositories_page(p)
        {
        }
    };
}

PackagesList::PackagesList(MainWindow * const m, PackagesPage * const p) :
    Gtk::TreeView(),
    PrivateImplementationPattern<PackagesList>(new Implementation<PackagesList>(m, p))
{
    set_model(_imp->filtered_model);

    append_column("Package", _imp->real_model->columns().col_package);

    Gtk::CellRendererText renderer;
    int c(append_column("Status", renderer) - 1);
    get_column(c)->add_attribute(renderer, "markup", _imp->real_model->columns().col_status_markup.index());

    append_column("Description", _imp->real_model->columns().col_description);

    signal_cursor_changed().connect(sigc::mem_fun(this, &PackagesList::handle_signal_cursor_changed));
}

PackagesList::~PackagesList()
{
}

void
PackagesList::handle_signal_cursor_changed()
{
    if (get_selection()->get_selected())
        _imp->repositories_page->set_qpn(paludis::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(
                        (*get_selection()->get_selected())[_imp->real_model->columns().col_pde].operator
                        paludis::tr1::shared_ptr<const PackageDatabaseEntry>()->name)));
    else
        _imp->repositories_page->set_qpn(paludis::tr1::shared_ptr<QualifiedPackageName>());
}

void
PackagesList::populate_real()
{
    _imp->real_model->populate();
    _imp->main_window->paludis_thread_action(
            sigc::mem_fun(this, &PackagesList::populate_in_paludis_thread), "Populating packages list");
}

void
PackagesList::populate_filter()
{
    _imp->filtered_model->populate();
}

void
PackagesList::populate_in_paludis_thread()
{
    _imp->main_window->gui_thread_action(
            sigc::mem_fun(this, &PackagesList::populate_in_gui_thread));
}

void
PackagesList::populate_in_gui_thread()
{
    expand_all();
    columns_autosize();

    if (! get_selection()->get_selected())
        if (! _imp->filtered_model->children().empty())
            set_cursor(_imp->filtered_model->get_path(_imp->filtered_model->children().begin()));
}

