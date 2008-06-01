/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_text_filter_source.hh"
#include "packages_text_filter_source_model.hh"
#include "packages_page.hh"
#include <paludis/filter.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesTextFilterSource>
    {
        Glib::RefPtr<PackagesTextFilterSourceModel> model;
        MainWindow * const main_window;
        PackagesPage * const packages_page;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            model(new PackagesTextFilterSourceModel(m)),
            main_window(m),
            packages_page(p)
        {
        }
    };
}

PackagesTextFilterSource::PackagesTextFilterSource(MainWindow * const m, PackagesPage * const p) :
    PrivateImplementationPattern<PackagesTextFilterSource>(new Implementation<PackagesTextFilterSource>(m, p)),
    Gtk::ComboBox()
{
    set_model(_imp->model);
    Gtk::CellRendererText renderer;
    pack_start(renderer);
    add_attribute(renderer, "text", _imp->model->columns().col_text.index());

    signal_changed().connect(sigc::mem_fun(this, &PackagesTextFilterSource::handle_signal_changed));
}

PackagesTextFilterSource::~PackagesTextFilterSource()
{
}

void
PackagesTextFilterSource::populate()
{
    _imp->model->populate();
    set_active(0);
}

void
PackagesTextFilterSource::handle_signal_changed()
{
    if (get_active())
        _imp->packages_page->set_text_filter((*get_active())[_imp->model->columns().col_filter]);
}

