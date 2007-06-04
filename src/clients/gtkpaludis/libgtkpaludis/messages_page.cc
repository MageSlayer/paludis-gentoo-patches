/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "messages_page.hh"
#include "main_notebook.hh"

#include <paludis/query.hh>
#include <paludis/util/fd_output_stream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/log.hh>
#include <gtkmm/scrolledwindow.h>
#include <vtemm/terminal_widget.hh>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<MessagesPage>
    {
        MainWindow * const main_window;
        MainNotebook * const main_notebook;

        Gtk::ScrolledWindow terminal_scroll;
        Vte::Terminal terminal;

        int master_fd, slave_fd;

        paludis::tr1::shared_ptr<FDOutputStream> messages_stream;

        sigc::connection terminal_cursor_moved_connection;

        Implementation(MainWindow * const m, MessagesPage * const, MainNotebook * const n) :
            main_window(m),
            main_notebook(n),
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

MessagesPage::MessagesPage(MainWindow * const m, MainNotebook * const n) :
    Gtk::Table(1, 1),
    MainNotebookPage(),
    PrivateImplementationPattern<MessagesPage>(new Implementation<MessagesPage>(m, this, n))
{
    _imp->terminal.set_pty(dup(_imp->master_fd));
    set_run_command_stdout_fds(_imp->slave_fd, _imp->master_fd);
    set_run_command_stderr_fds(_imp->slave_fd, _imp->master_fd);
    PStream::set_stderr_fd(_imp->slave_fd, _imp->master_fd);
    Log::get_instance()->set_log_stream(_imp->messages_stream.get());

    _imp->terminal.set_scroll_on_output(true);

    _imp->terminal_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
    _imp->terminal_scroll.add(_imp->terminal);
    attach(_imp->terminal_scroll, 0, 1, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 4, 4);

    _imp->terminal_cursor_moved_connection = _imp->terminal.signal_cursor_moved().connect(
            sigc::mem_fun(this, &MessagesPage::handle_terminal_cursor_moved));
}

MessagesPage::~MessagesPage()
{
    _imp->terminal_cursor_moved_connection.disconnect();
    Log::get_instance()->set_log_stream(&std::cerr);
}

void
MessagesPage::populate()
{
}

void
MessagesPage::handle_terminal_cursor_moved()
{
    _imp->main_notebook->mark_messages_page();
}

