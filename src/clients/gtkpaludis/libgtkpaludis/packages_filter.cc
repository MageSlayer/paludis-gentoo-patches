/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_filter.hh"
#include "packages_repository_filter.hh"
#include "packages_package_filter.hh"
#include "packages_text_filter.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/expander.h>
#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesFilter>
    {
        MainWindow * const main_window;
        PackagesPage * const packages_page;
        PackagesRepositoryFilter packages_repository_filter;
        PackagesPackageFilter packages_package_filter;
        PackagesTextFilter packages_text_filter;

        Gtk::Expander standard_expander;
        Gtk::Expander extra_expander;
        Gtk::Table standard_table;
        Gtk::Table extra_table;

        Gtk::Label extra_label;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            main_window(m),
            packages_page(p),
            packages_repository_filter(m, p),
            packages_package_filter(m, p),
            packages_text_filter(m, p),
            standard_expander("Filters:"),
            extra_expander("More filters:"),
            standard_table(3, 1),
            extra_table(1, 1),
            extra_label("EXTRA STUFF")
        {
        }
    };
}

PackagesFilter::PackagesFilter(MainWindow * const m, PackagesPage * const p) :
    Gtk::VBox(),
    PrivateImplementationPattern<PackagesFilter>(new Implementation<PackagesFilter>(m, p))
{
    add(_imp->standard_expander);
    _imp->standard_expander.add(_imp->standard_table);
    _imp->standard_expander.set_expanded(true);
    _imp->standard_table.attach(_imp->packages_repository_filter, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL, 4, 4);
    _imp->standard_table.attach(_imp->packages_package_filter, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL, 4, 4);
    _imp->standard_table.attach(_imp->packages_text_filter, 2, 3, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::FILL, 0, 0);

    add(_imp->extra_expander);
    _imp->extra_expander.add(_imp->extra_table);
    _imp->extra_table.attach(_imp->extra_label, 0, 1, 0, 1);
}

PackagesFilter::~PackagesFilter()
{
}

void
PackagesFilter::populate()
{
    _imp->packages_repository_filter.populate();
    _imp->packages_package_filter.populate();
    _imp->packages_text_filter.populate();
}

