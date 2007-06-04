/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_text_filter.hh"
#include "packages_text_filter_source.hh"
#include "packages_page.hh"
#include "main_window.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesTextFilter>
    {
        MainWindow * const main_window;
        PackagesPage * const packages_page;

        PackagesTextFilterSource packages_text_filter_source;
        Gtk::Entry filter_entry;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            main_window(m),
            packages_page(p),
            packages_text_filter_source(m, p)
        {
        }
    };
}

PackagesTextFilter::PackagesTextFilter(MainWindow * const m, PackagesPage * const p) :
    PrivateImplementationPattern<PackagesTextFilter>(new Implementation<PackagesTextFilter>(m, p)),
    Gtk::Table(2, 1)
{
    attach(_imp->packages_text_filter_source, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL, 4, 4);
    attach(_imp->filter_entry, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::FILL, 4, 4);

    _imp->filter_entry.signal_changed().connect(
            sigc::mem_fun(this, &PackagesTextFilter::handle_filter_entry_changed));
}

PackagesTextFilter::~PackagesTextFilter()
{
}

void
PackagesTextFilter::populate()
{
    _imp->packages_text_filter_source.populate();
}

void
PackagesTextFilter::handle_filter_entry_changed()
{
    _imp->packages_page->set_text_filter_text(_imp->filter_entry.get_text().raw());
}

