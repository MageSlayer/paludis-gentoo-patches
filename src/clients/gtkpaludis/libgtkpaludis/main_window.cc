/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "main_window.hh"
#include "main_notebook.hh"
#include <gtkmm/statusbar.h>
#include <gtkmm/table.h>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<MainWindow>
    {
        Gtk::Table main_table;
        MainNotebook main_notebook;
        Gtk::Statusbar status_bar;

        Implementation(MainWindow * const m) :
            main_table(2, 1),
            main_notebook(m)
        {
        }
    };
}

MainWindow::MainWindow(Environment * const e) :
    ThreadedWindow(Gtk::WINDOW_TOPLEVEL, e),
    PrivateImplementationPattern<MainWindow>(new Implementation<MainWindow>(this)),
    _imp(PrivateImplementationPattern<MainWindow>::_imp)
{
    set_title("gtkPaludis");
    set_default_size(800, 600);
    set_border_width(4);

    _imp->main_table.attach(_imp->main_notebook, 0, 1, 0, 1, Gtk::EXPAND | Gtk::FILL,
            Gtk::EXPAND | Gtk::FILL, 4, 4);
    _imp->main_table.attach(_imp->status_bar, 0, 1, 1, 2, Gtk::EXPAND | Gtk::FILL,
            Gtk::FILL, 4, 4);

    add(_imp->main_table);
    show_all();

    _imp->status_bar.set_has_resize_grip(true);
}

MainWindow::~MainWindow()
{
}

void
MainWindow::push_status_message(const std::string & s)
{
    _imp->status_bar.push(s);
}

void
MainWindow::pop_status_message()
{
    _imp->status_bar.pop();
}

void
MainWindow::do_set_sensitive(const bool v)
{
    _imp->main_notebook.set_sensitive(v);
}

MainNotebook *
MainWindow::main_notebook()
{
    return &_imp->main_notebook;
}

void
MainWindow::set_capture_output_options()
{
    _imp->main_notebook.set_capture_output_options();
}

