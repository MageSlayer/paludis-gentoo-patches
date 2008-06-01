/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_package_filter.hh"
#include "packages_package_filter_model.hh"
#include "packages_page.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesPackageFilter>
    {
        Glib::RefPtr<PackagesPackageFilterModel> model;
        MainWindow * const main_window;
        PackagesPage * const packages_page;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            model(new PackagesPackageFilterModel(m)),
            main_window(m),
            packages_page(p)
        {
        }
    };
}

PackagesPackageFilter::PackagesPackageFilter(MainWindow * const m, PackagesPage * const p) :
    PrivateImplementationPattern<PackagesPackageFilter>(new Implementation<PackagesPackageFilter>(m, p)),
    Gtk::ComboBox()
{
    set_model(_imp->model);
    Gtk::CellRendererText renderer;
    pack_start(renderer);
    add_attribute(renderer, "text", _imp->model->columns().col_text.index());

    signal_changed().connect(sigc::mem_fun(this, &PackagesPackageFilter::handle_signal_changed));
}

PackagesPackageFilter::~PackagesPackageFilter()
{
}

void
PackagesPackageFilter::populate()
{
    _imp->model->populate();
    set_active(0);
}

void
PackagesPackageFilter::handle_signal_changed()
{
    if (get_active())
        _imp->packages_page->set_package_filter((*get_active())[_imp->model->columns().col_filter]);
}


