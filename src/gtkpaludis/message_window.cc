/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "message_window.hh"
#include <gtkmm/main.h>
#include <paludis/util/log.hh>
#include <unistd.h>
#include <fcntl.h>

using namespace paludis;

namespace
{
    class Pipe :
        InstantiationPolicy<Pipe, instantiation_method::NonCopyableTag>
    {
        private:
            int _fds[2];

        public:
            Pipe()
            {
                if (-1 == pipe(_fds))
                    throw InternalError(PALUDIS_HERE, "pipe(2) failed");
            }

            ~Pipe()
            {
                close(_fds[0]);
                close(_fds[1]);
            }

            int read_fd() const
            {
                return _fds[0];
            }

            int write_fd() const
            {
                return _fds[1];
            }
    };

    /**
     * Output stream buffer class that's opened via an FD.
     *
     * See \ref TCppSL Ch. 13.13 for what we're doing here. The buffer code is
     * based upon the "io/outbuf2.hpp" example in section 13.13.3.
     */
    class FDOutputStreamBuf :
        public std::streambuf
    {
        protected:
            int fd;

            virtual int_type
            overflow(int_type c)
            {
                if (c != EOF)
                {
                    char z = c;
                    if (1 != write(fd, &z, 1))
                        return EOF;
                }
                return c;
            }

            virtual std::streamsize
            xsputn(const char * s, std::streamsize num)
            {
                return write(fd, s, num);
            }

        public:
            FDOutputStreamBuf(const int f) :
                fd(f)
            {
            }
    };

    /**
     * Member from base initialisation for FDOutputStream.
     */
    class FDOutputStreamBase
    {
        protected:
            FDOutputStreamBuf buf;

        public:
            FDOutputStreamBase(const int fd) :
                buf(fd)
            {
            }
    };

    /**
     * Output stream buffer class that's opened via an FD.
     */
    class FDOutputStream :
        protected FDOutputStreamBase,
        public std::ostream
    {
        public:
            FDOutputStream(const int fd) :
                FDOutputStreamBase(fd),
                std::ostream(&buf)
        {
        }
    };
}

namespace paludis
{
    template<>
    struct Implementation<MessageWindow> :
        InternalCounted<Implementation<MessageWindow> >
    {
        Pipe log_pipe;
        MessageWindow * const owner;
        Glib::RefPtr<Glib::IOChannel> log_connection;
        FDOutputStream stream;

        Implementation(MessageWindow * const o) :
            owner(o),
            log_connection(Glib::IOChannel::create_from_fd(log_pipe.read_fd())),
            stream(log_pipe.write_fd())
        {
            Glib::signal_io().connect(sigc::mem_fun(*owner, &MessageWindow::on_log_read),
                    log_pipe.read_fd(), Glib::IO_IN);
            Log::get_instance()->set_log_stream(&stream);
            Log::get_instance()->message(ll_debug, lc_no_context, "Message window initialised");
        }
    };
}

MessageWindow::MessageWindow() :
    PrivateImplementationPattern<MessageWindow>(new Implementation<MessageWindow>(this))
{
    set_editable(false);
}

MessageWindow::~MessageWindow()
{
}

bool
MessageWindow::on_log_read(Glib::IOCondition io_condition)
{
    if (0 == io_condition & Glib::IO_IN)
        return false;

    Glib::ustring buf;

#ifndef GLIBMM_EXCEPTIONS_ENABLED
#  error Your glibcc hates us
#endif

    _imp->log_connection->read_line(buf);
    get_buffer()->insert(get_buffer()->end(), buf);

    return true;
}

