/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
 * Copyright (c) 2006 Piotr Rak <piotr.rak@gmail.com>
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
#include "vte_message_window.hh"
#include <paludis/util/fd_output_stream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/exception.hh>

//#ifndef _XOPEN_SOURCE
//#define _XOPEN_SOURCE don't know why vte.h has this defined
//#endif

#include <cstdlib>
#include <fcntl.h>
#include <iostream>

namespace
{
    class Pty /* this one should propably go to libpaludisutil */
    {
        int _master_fd;
        int _slave_fd;
    public:
        Pty(bool controlling_terminal);
        ~Pty();

        int master_fd() const;
        int slave_fd() const;
    };

    Pty::Pty(bool controlling_terminal) :
        _master_fd(posix_openpt(O_RDWR | (controlling_terminal ? 0 : O_NOCTTY)))
    {
        if (0 > _master_fd)
            throw paludis::InternalError(PALUDIS_HERE, "posix_openpt failed");

        char *slave_name(ptsname(_master_fd));

        try
        {
            if (0 == slave_name)
                throw paludis::InternalError(PALUDIS_HERE, "ptsname failed");

            if (0 > grantpt(_master_fd)) /* TODO: this one needs something special from user co */
                throw paludis::InternalError(PALUDIS_HERE, "grantpt failed");

            if (0 > unlockpt(_master_fd))
                throw paludis::InternalError(PALUDIS_HERE, "unlockpt failed");

            if (-1 == (_slave_fd = open(slave_name, O_RDWR)))
                throw paludis::InternalError(PALUDIS_HERE, "open slave terminal failed");
        }
        catch (...)
        {
            close(_master_fd);
            throw;
        }
    }

    Pty::~Pty()
    {
        close(_master_fd);
        close(_slave_fd);
    }

    int Pty::master_fd() const
    {
        return _master_fd;
    }

    int Pty::slave_fd() const
    {
        return _slave_fd;
    }

} /* anonymous namespace */

namespace paludis
{
    template <>
    struct Implementation<VteMessageWindow> :
        public InternalCounted<Implementation<VteMessageWindow> >
    {
        VteMessageWindow * const owner;
        Pty pty;
        FDOutputStream stream;

        Implementation(VteMessageWindow *o);

    };

    Implementation<VteMessageWindow>::Implementation(VteMessageWindow* o) :
        InternalCounted<Implementation<VteMessageWindow> >(),
        owner(o),
        pty(false),
        stream(pty.slave_fd())
    {
        set_run_command_stdout_fds(pty.slave_fd(), pty.master_fd());
        set_run_command_stderr_fds(pty.slave_fd(), pty.master_fd());

        Log::get_instance()->set_log_stream(&stream);
        Log::get_instance()->message(ll_debug, lc_no_context, "Message window initialized");
    }


    VteMessageWindow::VteMessageWindow() :
        Vte::Terminal(),
        PrivateImplementationPattern<VteMessageWindow>(new Implementation<VteMessageWindow>(this))
    {
        set_pty(dup(_imp->pty.master_fd()));
    }

    VteMessageWindow::~VteMessageWindow()
    {
        Log::get_instance()->set_log_stream(&std::cerr);
    }

} /* namespace paludis */

