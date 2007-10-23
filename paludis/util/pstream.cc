/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/util/log.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/stringify.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <cstring>
#include <iostream>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <grp.h>

using namespace paludis;

PStreamError::PStreamError(const std::string & our_message) throw () :
    Exception(our_message)
{
}

PStreamInBuf::int_type
PStreamInBuf::underflow()
{
    if (gptr() < egptr())
        return *gptr();

    int num_putback = gptr() - eback();
    if (num_putback > putback_size)
        num_putback = putback_size;
    std::memmove(buffer + putback_size - num_putback,
            gptr() - num_putback, num_putback);

    ssize_t n = read(fd, buffer + putback_size, buffer_size - putback_size);
    if (n == 0)
        return EOF;
    else if (n < 0)
        throw PStreamError("read returned error " + stringify(strerror(errno)) + ", fd is " + stringify(fd));

    setg(buffer + putback_size - num_putback, buffer + putback_size, buffer + putback_size + n);

    return *gptr();
}

PStreamInBuf::PStreamInBuf(const Command & cmd) :
    _command(cmd)
{
    Context context("When running command '" + stringify(cmd.command()) + "' asynchronously:");

    std::string extras;

    if (! cmd.chdir().empty())
        extras.append(" [chdir " + cmd.chdir() + "]");

    for (Command::ConstIterator s(cmd.begin_setenvs()), s_end(cmd.end_setenvs()) ; s != s_end ; ++s)
        extras.append(" [setenv " + s->first + "=" + s->second + "]");

    if (cmd.gid() && *cmd.gid() != getgid())
        extras.append(" [setgid " + stringify(*cmd.gid()) + "]");

    if (cmd.uid() && *cmd.uid() != getuid())
        extras.append(" [setuid " + stringify(*cmd.uid()) + "]");

    extras.append(" [stdout_pipe " + stringify(stdout_pipe.read_fd()) + ", " + stringify(stdout_pipe.write_fd()) + "]");

    std::string c(cmd.command());

    if ((! cmd.stdout_prefix().empty()) || (! cmd.stderr_prefix().empty()))
        c = getenv_with_default("PALUDIS_OUTPUTWRAPPER_DIR", LIBEXECDIR "/paludis/utils") + "/outputwrapper --stdout-prefix '"
            + cmd.stdout_prefix() + "' --stderr-prefix '" + cmd.stderr_prefix() + "' "
            + (cmd.prefix_discard_blank_output() ? " --discard-blank-output " : "")
            + (cmd.prefix_blank_lines() ? " --wrap-blanks " : "")
            + " -- " + c;

    cmd.echo_to_stderr();
    Log::get_instance()->message(ll_debug, lc_context, "execl /bin/sh -c " + c + " " + extras);

    child = fork();

    if (0 == child)
    {
        try
        {
            if (! cmd.chdir().empty())
                if (-1 == chdir(cmd.chdir().c_str()))
                    throw RunCommandError("chdir failed: " + stringify(strerror(errno)));

            for (Command::ConstIterator s(cmd.begin_setenvs()), s_end(cmd.end_setenvs()) ; s != s_end ; ++s)
                setenv(s->first.c_str(), s->second.c_str(), 1);

            if (-1 == dup2(stdout_pipe.write_fd(), 1))
                throw PStreamError("dup2 failed");
            close(stdout_pipe.read_fd());

            if (-1 != PStream::stderr_fd)
            {
                if (-1 == dup2(PStream::stderr_fd, 2))
                    throw PStreamError("dup2 failed");

                if (-1 != PStream::stderr_close_fd)
                    close(PStream::stderr_close_fd);
            }

            if (cmd.gid() && *cmd.gid() != getgid())
            {
                gid_t g(*cmd.gid());

                if (0 != ::setgid(*cmd.gid()))
                    std::cerr << "setgid(" << *cmd.gid() << ") failed for exec of '" << c << "': "
                        << strerror(errno) << std::endl;
                else if (0 != ::setgroups(1, &g))
                    std::cerr << "setgroups failed for exec of '" << c << "': " << strerror(errno) << std::endl;
            }

            if (cmd.uid() && *cmd.uid() != getuid())
                if (0 != ::setuid(*cmd.uid()))
                    std::cerr << "setuid(" << *cmd.uid() << ") failed for exec of '" << c << "': "
                        << strerror(errno) << std::endl;

            execl("/bin/sh", "sh", "-c", c.c_str(), static_cast<char *>(0));
            throw PStreamError("execl /bin/sh -c '" + c + "' failed:"
                    + stringify(strerror(errno)));
        }
        catch (const Exception & e)
        {
            std::cerr << "exec of '" << c << "' failed due to exception '" << e.message()
                << "' (" << e.what() << ")" << std::endl;
            exit(123);
        }
        catch (...)
        {
            std::cerr << "exec of '" << c << "' failed due to unknown exception" << std::endl;
            exit(124);
        }
    }
    else if (-1 == child)
        throw PStreamError("fork failed: " + stringify(strerror(errno)));
    else
    {
        close(stdout_pipe.write_fd());
        stdout_pipe.clear_write_fd();
        fd = stdout_pipe.read_fd();
    }

    setg(buffer + putback_size, buffer + putback_size, buffer + putback_size);
}

PStreamInBuf::~PStreamInBuf()
{
    Context context("When destroying PStream process with fd '" + stringify(fd) + "':");

    if (0 != fd)
    {
        int fdn(fd), x;
        if (-1 == waitpid(child, &x, 0))
            throw PStreamError("waitpid returned error " + stringify(strerror(errno)) + ", fd is " + stringify(fd));
        Log::get_instance()->message(ll_debug, lc_context) << "waitpid " << fdn << " for destructor -> " <<
            (WIFSIGNALED(x) ? "signal " + stringify(WTERMSIG(x) + 128) : "exit status " + stringify(WEXITSTATUS(x)));
    }
}

int
PStreamInBuf::exit_status()
{
    Context context("When requesting exit status for PStream process with fd '" + stringify(fd) + "':");
    if (0 != fd)
    {
        int fdn(fd);
        if (-1 == waitpid(child, &_exit_status, 0))
            throw PStreamError("waitpid returned error " + stringify(strerror(errno)) + ", fd is " + stringify(fd));
        fd = 0;
        Log::get_instance()->message(ll_debug, lc_context) << "waitpid " << fdn << " for exit_status() -> " <<
            (WIFSIGNALED(_exit_status) ? "signal " + stringify(WTERMSIG(_exit_status) + 128) : "exit status " + stringify(WEXITSTATUS(_exit_status)));
    }
    return WIFSIGNALED(_exit_status) ? WTERMSIG(_exit_status) + 128 : WEXITSTATUS(_exit_status);
}

void
PStream::set_stderr_fd(const int fd, const int fd2)
{
    _stderr_fd = fd;
    _stderr_close_fd = fd2;
}

int PStream::_stderr_fd(-1);
const int & PStream::stderr_fd(_stderr_fd);

int PStream::_stderr_close_fd(-1);
const int & PStream::stderr_close_fd(_stderr_close_fd);

