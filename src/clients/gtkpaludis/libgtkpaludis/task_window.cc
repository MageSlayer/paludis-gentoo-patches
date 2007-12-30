/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "task_window.hh"
#include "main_window.hh"
#include "gui_task.hh"
#include "task_sequence_list.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/fd_output_stream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/scrolledwindow.h>
#include <vtemm/terminal_widget.hh>
#include <cstdlib>
#include <fcntl.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<TaskWindow>
    {
        MainWindow * const main_window;
        GuiTask * const task;

        Gtk::Table main_table;
        Gtk::HButtonBox main_button_box;
        Gtk::Button ok_button;
        Gtk::Statusbar status_bar;

        Gtk::ScrolledWindow task_sequence_list_scroll;
        TaskSequenceList task_sequence_list;

        Vte::Terminal terminal;
        int master_fd, slave_fd;

        paludis::tr1::shared_ptr<FDOutputStream> messages_stream;

        Implementation(MainWindow * const m, GuiTask * const t) :
            main_window(m),
            task(t),
            main_table(4, 1),
            ok_button(Gtk::Stock::OK),
            master_fd(posix_openpt(O_RDWR | O_NOCTTY))
        {
            grantpt(master_fd);
            unlockpt(master_fd);
            slave_fd = open(ptsname(master_fd), O_RDWR);

            messages_stream.reset(new FDOutputStream(slave_fd));
        }

        ~Implementation()
        {
            close(master_fd);
            close(slave_fd);
        }
    };
}

TaskWindow::TaskWindow(MainWindow * const m, GuiTask * const t) :
    ThreadedWindow(Gtk::WINDOW_TOPLEVEL, m->environment()),
    PrivateImplementationPattern<TaskWindow>(new Implementation<TaskWindow>(m, t)),
    _imp(PrivateImplementationPattern<TaskWindow>::_imp)
{
    m->desensitise();

    set_modal(true);
    set_transient_for(*m);
    set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    set_title("Task");
    set_default_size(700, 500);
    set_border_width(4);

    _imp->ok_button.signal_clicked().connect(sigc::mem_fun(this, &TaskWindow::handle_ok_button_clicked));

    _imp->terminal.set_pty(dup(_imp->master_fd));
    set_run_command_stdout_fds(_imp->slave_fd, _imp->master_fd);
    set_run_command_stderr_fds(_imp->slave_fd, _imp->master_fd);
    Log::get_instance()->set_log_stream(_imp->messages_stream.get());

    _imp->terminal.set_scroll_on_output(true);

    _imp->main_button_box.set_layout(Gtk::BUTTONBOX_END);
    _imp->main_button_box.set_spacing(10);
    _imp->main_button_box.add(_imp->ok_button);

    _imp->status_bar.set_has_resize_grip(true);

    _imp->task_sequence_list_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _imp->task_sequence_list_scroll.add(_imp->task_sequence_list);
    _imp->main_table.attach(_imp->task_sequence_list_scroll, 0, 1, 0, 1, Gtk::EXPAND | Gtk::FILL,
            Gtk::EXPAND | Gtk::FILL, 4, 4);

    _imp->main_table.attach(_imp->terminal, 0, 1, 1, 2, Gtk::EXPAND | Gtk::FILL,
            Gtk::EXPAND | Gtk::FILL, 4, 4);

    _imp->main_table.attach(_imp->main_button_box, 0, 1, 2, 3, Gtk::EXPAND | Gtk::FILL,
            Gtk::FILL, 4, 4);
    _imp->main_table.attach(_imp->status_bar, 0, 1, 3, 4, Gtk::EXPAND | Gtk::FILL,
            Gtk::FILL, 4, 4);

    add(_imp->main_table);

    show_all();
}

TaskWindow::~TaskWindow()
{
    _imp->main_window->set_capture_output_options();
    _imp->main_window->sensitise();
}

void
TaskWindow::handle_ok_button_clicked()
{
    handle_delete_event(0);
}

bool
TaskWindow::handle_delete_event(GdkEventAny * a)
{
    ThreadedWindow::handle_delete_event(a);
    delete _imp->task;
    delete this;
    return false;
}

void
TaskWindow::push_status_message(const std::string & s)
{
    _imp->status_bar.push(s);
}

void
TaskWindow::pop_status_message()
{
    _imp->status_bar.pop();
}

void
TaskWindow::do_set_sensitive(const bool v)
{
    _imp->main_button_box.set_sensitive(v);
}

void
TaskWindow::run()
{
    show_all();
    paludis_thread_action(sigc::mem_fun(_imp->task, &GuiTask::paludis_thread_execute), "Executing task");
}

void
TaskWindow::append_sequence_item(const std::string & id, const std::string & description,
        const std::string & status)
{
    _imp->task_sequence_list.append_sequence_item(id, description, status);
}

void
TaskWindow::set_sequence_item_status(const std::string & id, const std::string & status)
{
    _imp->task_sequence_list.set_sequence_item_status(id, status);
}

