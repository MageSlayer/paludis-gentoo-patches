/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <cstdlib>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/destringify.hh>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <map>
#include <iostream>
#include "config.h"

/** \file
 * Implementation of various system utilities.
 *
 * \ingroup grpsystem
 */

using namespace paludis;

namespace
{
    static int stdout_write_fd = -1;
    static int stdout_close_fd = -1;

    static int stderr_write_fd = -1;
    static int stderr_close_fd = -1;

    pid_t get_paludis_pid()
    {
        pid_t result(0);
        std::string str;

        if (((str = getenv_with_default("PALUDIS_PID", ""))).empty())
        {
            result = getpid();
            setenv("PALUDIS_PID", stringify(result).c_str(), 1);
        }
        else
            result = destringify<pid_t>(str);

        return result;
    }


    static pid_t paludis_pid(get_paludis_pid());
}

void
paludis::set_run_command_stdout_fds(const int w, const int c)
{
    stdout_write_fd = w;
    stdout_close_fd = c;
}

void
paludis::set_run_command_stderr_fds(const int w, const int c)
{
    stderr_write_fd = w;
    stderr_close_fd = c;
}

GetenvError::GetenvError(const std::string & key) throw () :
    Exception("Environment variable '" + key + "' not set")
{
}

RunCommandError::RunCommandError(const std::string & our_message) throw () :
    Exception(our_message)
{
}

std::string
paludis::getenv_with_default(const std::string & key, const std::string & def)
{
    const char * const e(std::getenv(key.c_str()));
    return e ? e : def;
}

std::string
paludis::getenv_or_error(const std::string & key)
{
    const char * const e(std::getenv(key.c_str()));
    if (! e)
        throw GetenvError(key);
    return e;
}

namespace
{
    /**
     * Fetch the kernel version, for paludis::kernel_version.
     *
     * \ingroup grpsystem
     */
    std::string get_kernel_version()
    {
        struct utsname u;
        if (0 != uname(&u))
            throw InternalError(PALUDIS_HERE, "uname call failed");
        return u.release;
    }
}

std::string
paludis::kernel_version()
{
    static const std::string result(get_kernel_version());
    return result;
}

namespace paludis
{
    template<>
    struct Implementation<Command>
    {
        std::string command;
        std::map<std::string, std::string> setenv_values;
        std::string chdir;
        bool echo_to_stderr;
        std::tr1::shared_ptr<uid_t> uid;
        std::tr1::shared_ptr<gid_t> gid;

        Implementation(const std::string & c,
                const std::map<std::string, std::string> & s = (std::map<std::string, std::string>()),
                const std::string & d = "", bool e = false, std::tr1::shared_ptr<uid_t> u = std::tr1::shared_ptr<uid_t>(),
                std::tr1::shared_ptr<gid_t> g = std::tr1::shared_ptr<gid_t>()) :
            command(c),
            setenv_values(s),
            chdir(d),
            echo_to_stderr(e),
            uid(u),
            gid(g)
        {
        }
    };
}

Command::Command(const std::string & s) :
    PrivateImplementationPattern<Command>(new Implementation<Command>(s))
{
}

Command::Command(const char * const s) :
    PrivateImplementationPattern<Command>(new Implementation<Command>(s))
{
}

Command::Command(const Command & other) :
    PrivateImplementationPattern<Command>(new Implementation<Command>(other._imp->command,
                other._imp->setenv_values, other._imp->chdir, other._imp->echo_to_stderr,
                other._imp->uid, other._imp->gid))
{
}

const Command &
Command::operator= (const Command & other)
{
    if (this != &other)
    {
        _imp.reset(new Implementation<Command>(other._imp->command, other._imp->setenv_values,
                    other._imp->chdir, other._imp->echo_to_stderr));
        if (other.uid() && other.gid())
            with_uid_gid(*other.uid(), *other.gid());
    }

    return *this;
}

Command::~Command()
{
}

Command &
Command::with_chdir(const FSEntry & c)
{
    _imp->chdir = stringify(c);
    return *this;
}

Command &
Command::with_setenv(const std::string & k, const std::string & v)
{
    _imp->setenv_values.insert(std::make_pair(k, v));
    return *this;
}

Command &
Command::with_uid_gid(const uid_t u, const gid_t g)
{
    _imp->uid.reset(new uid_t(u));
    _imp->gid.reset(new gid_t(g));
    return *this;
}

Command &
Command::with_sandbox()
{
#if HAVE_SANDBOX
    if (! getenv_with_default("PALUDIS_DO_NOTHING_SANDBOXY", "").empty())
        Log::get_instance()->message(ll_debug, lc_no_context,
                "PALUDIS_DO_NOTHING_SANDBOXY is set, not using sandbox");
    else if (! getenv_with_default("SANDBOX_ACTIVE", "").empty())
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Already inside sandbox, not spawning another sandbox instance");
    else
        _imp->command = "sandbox " + _imp->command;
#endif

    return *this;
}

std::tr1::shared_ptr<const uid_t>
Command::uid() const
{
    return _imp->uid;
}

std::tr1::shared_ptr<const gid_t>
Command::gid() const
{
    return _imp->gid;
}

int
paludis::run_command(const Command & cmd)
{
    Context context("When running command '" + stringify(cmd.command()) + "':");

    pid_t child(fork());
    if (0 == child)
    {
        std::string extras;

        if (! cmd.chdir().empty())
        {
            if (-1 == chdir(stringify(cmd.chdir()).c_str()))
                throw RunCommandError("chdir failed: " + stringify(strerror(errno)));
            extras.append(" [chdir " + cmd.chdir() + "]");
        }

        for (Command::Iterator s(cmd.begin_setenvs()), s_end(cmd.end_setenvs()) ; s != s_end ; ++s)
        {
            setenv(s->first.c_str(), s->second.c_str(), 1);
            extras.append(" [setenv " + s->first + "=" + s->second + "]");
        }

        if (-1 != stdout_write_fd)
        {
            if (-1 == dup2(stdout_write_fd, 1))
                throw RunCommandError("dup2 failed");

            if (-1 != stdout_close_fd)
                close(stdout_close_fd);
        }

        if (-1 != stderr_write_fd)
        {
            if (-1 == dup2(stderr_write_fd, 2))
                throw RunCommandError("dup2 failed");

            if (-1 != stderr_close_fd)
                close(stderr_close_fd);
        }

        if (cmd.gid() && *cmd.gid() != getgid())
        {
            gid_t g(*cmd.gid());

            if (0 != ::setgid(*cmd.gid()))
                Log::get_instance()->message(ll_warning, lc_context, "setgid("
                        + stringify(*cmd.gid()) + ") failed: " + stringify(strerror(errno)));
            else if (0 != ::setgroups(1, &g))
                Log::get_instance()->message(ll_warning, lc_context, "setgroups failed: "
                        + stringify(strerror(errno)));

            extras.append(" [setgid " + stringify(*cmd.gid()) + "]");
        }

        if (cmd.uid() && *cmd.uid() != getuid())
        {
            if (0 != ::setuid(*cmd.uid()))
                Log::get_instance()->message(ll_warning, lc_context, "setuid("
                        + stringify(*cmd.uid()) + ") failed: " + stringify(strerror(errno)));
            extras.append(" [setuid " + stringify(*cmd.uid()) + "]");
        }

        cmd.echo_to_stderr();
        Log::get_instance()->message(ll_debug, lc_no_context, "execl /bin/sh -c " + cmd.command()
                + " " + extras);
        execl("/bin/sh", "sh", "-c", cmd.command().c_str(), static_cast<char *>(0));
        throw RunCommandError("execl /bin/sh -c '" + cmd.command() + "' failed:"
                + stringify(strerror(errno)));
    }
    else if (-1 == child)
        throw RunCommandError("fork failed: " + stringify(strerror(errno)));
    else
    {
        int status(-1);
        if (-1 == wait(&status))
            throw RunCommandError("wait failed: " + stringify(strerror(errno)));
        return WEXITSTATUS(status);
    }

    throw InternalError(PALUDIS_HERE, "should never be reached");
}

std::string
Command::command() const
{
    return _imp->command;
}

std::string
Command::chdir() const
{
    return _imp->chdir;
}

Command::Iterator
Command::begin_setenvs() const
{
    return Iterator(_imp->setenv_values.begin());
}

Command::Iterator
Command::end_setenvs() const
{
    return Iterator(_imp->setenv_values.end());
}

void
Command::echo_to_stderr() const
{
    if (! _imp->echo_to_stderr)
        return;

    std::cerr << command().c_str() << std::endl;
}

Command &
Command::with_echo_to_stderr()
{
    _imp->echo_to_stderr = true;
    return *this;
}

