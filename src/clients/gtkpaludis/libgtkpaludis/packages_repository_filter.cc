/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_repository_filter.hh"
#include "packages_repository_filter_model.hh"
#include "packages_page.hh"
#include <paludis/query.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesRepositoryFilter>
    {
        Glib::RefPtr<PackagesRepositoryFilterModel> model;
        MainWindow * const main_window;
        PackagesPage * const packages_page;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            model(new PackagesRepositoryFilterModel(m)),
            main_window(m),
            packages_page(p)
        {
        }
    };
}

PackagesRepositoryFilter::PackagesRepositoryFilter(MainWindow * const m, PackagesPage * const p) :
    PrivateImplementationPattern<PackagesRepositoryFilter>(new Implementation<PackagesRepositoryFilter>(m, p)),
    Gtk::ComboBox()
{
    set_model(_imp->model);
    Gtk::CellRendererText renderer;
    pack_start(renderer);
    add_attribute(renderer, "text", _imp->model->columns().col_text.index());
    add_attribute(renderer, "sensitive", _imp->model->columns().col_sensitive.index());

    signal_changed().connect(sigc::mem_fun(this, &PackagesRepositoryFilter::handle_signal_changed));
}

PackagesRepositoryFilter::~PackagesRepositoryFilter()
{
}

void
PackagesRepositoryFilter::populate()
{
    _imp->model->populate();
    set_active(0);
}

void
PackagesRepositoryFilter::handle_signal_changed()
{
    if (get_active())
        _imp->packages_page->set_repository_filter((*get_active())[_imp->model->columns().col_query].operator
                std::tr1::shared_ptr<const Query>());
}

