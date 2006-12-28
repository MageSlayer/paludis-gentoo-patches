/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "messages.hh"
#include "main_window.hh"

#include <vtemm/terminal_widget.hh>
#include <paludis/util/fd_output_stream.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/pstream.hh>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

// for restoring Log's output stream
#include <iostream>

using namespace paludis;
using namespace gtkpaludis;

namespace
{
    class Pty
    {
        private:
            int _master_fd, _slave_fd;

        public:
            Pty() :
                _master_fd(posix_openpt(O_RDWR))
            {
                if (_master_fd < 1)
                    throw InternalError(PALUDIS_HERE, "posix_openpt failed");
                if (-1 == grantpt(_master_fd))
                    throw InternalError(PALUDIS_HERE, "grantpt failed");
                if (-1 == unlockpt(_master_fd))
                    throw InternalError(PALUDIS_HERE, "unlockpt failed");

                _slave_fd = open(ptsname(_master_fd), O_RDWR);
                if (-1 == _slave_fd)
                    throw InternalError(PALUDIS_HERE, "open _slave_fd failed");
            }

            ~Pty()
            {
                close(_master_fd);
                close(_slave_fd);
            }

            int master_fd() const
            {
                return _master_fd;
            }

            int slave_fd() const
            {
                return _slave_fd;
            }
    };
}

namespace paludis
{
    template<>
    struct Implementation<Messages> :
        InternalCounted<Implementation<Messages> >
    {
        Pty term_pty;
        Vte::Terminal term;
        FDOutputStream messages_stream;

        Implementation() :
            messages_stream(term_pty.slave_fd())
        {
        }
    };
}

Messages::Messages() :
    PrivateImplementationPattern<Messages>(new Implementation<Messages>)
{
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    add(_imp->term);

    _imp->term.set_pty(dup(_imp->term_pty.master_fd()));
    _imp->term.set_scroll_on_output(true);

    set_run_command_stdout_fds(_imp->term_pty.slave_fd(), _imp->term_pty.master_fd());
    set_run_command_stderr_fds(_imp->term_pty.slave_fd(), _imp->term_pty.master_fd());
    PStream::set_stderr_fd(_imp->term_pty.slave_fd(), _imp->term_pty.master_fd());

    Log::get_instance()->set_log_stream(&_imp->messages_stream);

    Glib::signal_idle().connect(sigc::bind_return(sigc::mem_fun(this,
                    &Messages::_install_signal_handlers), false));
}

Messages::~Messages()
{
    Log::get_instance()->set_log_stream(&std::cerr);
}

void
Messages::message(const std::string & s)
{
    std::string msg("=== " + s + " ===\n");
    write(_imp->term_pty.slave_fd(), msg.c_str(), msg.length());
}

void
Messages::_install_signal_handlers()
{
    _imp->term.signal_cursor_moved().connect(sigc::mem_fun(MainWindow::get_instance(),
                &MainWindow::message_window_changed));
}

