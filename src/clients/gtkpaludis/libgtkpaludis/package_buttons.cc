/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "package_buttons.hh"
#include "main_window.hh"
#include "query_window.hh"
#include "packages_page.hh"
#include <gtkmm/button.h>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackageButtons>
    {
        MainWindow * const main_window;
        PackagesPage * const packages_page;

        Gtk::Button install_button;
        Gtk::Button uninstall_button;
        Gtk::Button query_button;

        Implementation(MainWindow * const m, PackagesPage * const p) :
            main_window(m),
            packages_page(p),
            install_button("Install..."),
            uninstall_button("Uninstall..."),
            query_button("Query...")
        {
        }
    };
}

PackageButtons::PackageButtons(MainWindow * const m, PackagesPage * const p) :
    Gtk::HButtonBox(),
    PrivateImplementationPattern<PackageButtons>(new Implementation<PackageButtons>(m, p))
{
    set_layout(Gtk::BUTTONBOX_END);
    set_spacing(10);

    _imp->install_button.set_sensitive(false);
    add(_imp->install_button);

    _imp->uninstall_button.set_sensitive(false);
    add(_imp->uninstall_button);

    _imp->query_button.set_sensitive(false);
    _imp->query_button.signal_clicked().connect(sigc::mem_fun(this, &PackageButtons::handle_query_button_clicked));
    add(_imp->query_button);
}

PackageButtons::~PackageButtons()
{
}

void
PackageButtons::populate()
{
    _imp->main_window->paludis_thread_action(
            sigc::bind(sigc::mem_fun(this, &PackageButtons::populate_in_paludis_thread),
                _imp->packages_page->get_qpn()), "Updating package buttons");
}

void
PackageButtons::populate_in_paludis_thread(paludis::tr1::shared_ptr<const QualifiedPackageName> q)
{
    _imp->main_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &PackageButtons::populate_in_gui_thread),
                q, false, false));
}

void
PackageButtons::populate_in_gui_thread(const bool u, const bool v, const bool w)
{
    _imp->query_button.set_sensitive(u);
    _imp->install_button.set_sensitive(v);
    _imp->uninstall_button.set_sensitive(w);
}

void
PackageButtons::handle_query_button_clicked()
{
    QueryWindow * q(new QueryWindow(_imp->main_window, *_imp->packages_page->get_qpn()));
    q->show_all();
}

