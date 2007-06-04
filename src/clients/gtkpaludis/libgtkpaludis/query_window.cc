/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "query_window.hh"
#include "query_notebook.hh"
#include "main_window.hh"
#include <paludis/name.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>
#include <gtkmm/statusbar.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<QueryWindow>
    {
        MainWindow * const main_window;

        Gtk::Table main_table;
        Gtk::HButtonBox main_button_box;
        Gtk::Button ok_button;
        Gtk::Statusbar status_bar;

        QueryNotebook query_notebook;
        QualifiedPackageName package_name;

        Implementation(MainWindow * const m, QueryWindow * const q, const QualifiedPackageName & n) :
            main_window(m),
            main_table(3, 1),
            ok_button(Gtk::Stock::OK),
            query_notebook(q),
            package_name(n)
        {
        }
    };
}

QueryWindow::QueryWindow(MainWindow * const m, const QualifiedPackageName & q) :
    ThreadedWindow(Gtk::WINDOW_TOPLEVEL, m->environment()),
    PrivateImplementationPattern<QueryWindow>(new Implementation<QueryWindow>(m, this, q)),
    _imp(PrivateImplementationPattern<QueryWindow>::_imp)
{
    m->desensitise();

    set_modal(true);
    set_transient_for(*m);
    set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    set_title("Querying '" + stringify(q) + "'");
    set_default_size(700, 500);
    set_border_width(4);

    _imp->ok_button.signal_clicked().connect(sigc::mem_fun(this, &QueryWindow::handle_ok_button_clicked));

    _imp->main_button_box.set_layout(Gtk::BUTTONBOX_END);
    _imp->main_button_box.set_spacing(10);
    _imp->main_button_box.add(_imp->ok_button);

    _imp->status_bar.set_has_resize_grip(true);

    _imp->main_table.attach(_imp->query_notebook, 0, 1, 0, 1, Gtk::EXPAND | Gtk::FILL,
            Gtk::EXPAND | Gtk::FILL, 4, 4);
    _imp->main_table.attach(_imp->main_button_box, 0, 1, 1, 2, Gtk::EXPAND | Gtk::FILL,
            Gtk::FILL, 4, 4);
    _imp->main_table.attach(_imp->status_bar, 0, 1, 2, 3, Gtk::EXPAND | Gtk::FILL,
            Gtk::FILL, 4, 4);

    add(_imp->main_table);

    show_all();
}

QueryWindow::~QueryWindow()
{
    _imp->main_window->sensitise();
}

void
QueryWindow::handle_ok_button_clicked()
{
    handle_delete_event(0);
}

bool
QueryWindow::handle_delete_event(GdkEventAny * a)
{
    ThreadedWindow::handle_delete_event(a);
    delete this;
    return false;
}

const QualifiedPackageName
QueryWindow::get_package_name() const
{
    return _imp->package_name;
}

void
QueryWindow::push_status_message(const std::string & s)
{
    _imp->status_bar.push(s);
}

void
QueryWindow::pop_status_message()
{
    _imp->status_bar.pop();
}

void
QueryWindow::do_set_sensitive(const bool v)
{
    _imp->query_notebook.set_sensitive(v);
    _imp->main_button_box.set_sensitive(v);
}


