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

    /**
     * Runs a command in a directory if needed, wait for it to terminate
     * and return its exit status.
     *
     * \ingroup grpsystem
     */
    int
    real_run_command(const std::string & cmd, const FSEntry * const fsentry)
    {
        pid_t child(fork());
        if (0 == child)
        {
            if (fsentry)
                if (-1 == chdir(stringify(*fsentry).c_str()))
                    throw RunCommandError("chdir failed: " + stringify(strerror(errno)));

            if (-1 != stdout_write_fd)
            {
                Log::get_instance()->message(ll_debug, lc_no_context, "dup2 " +
                        stringify(stdout_write_fd) + " 2");

                if (-1 == dup2(stdout_write_fd, 1))
                    throw RunCommandError("dup2 failed");

                if (-1 != stdout_close_fd)
                        close(stdout_close_fd);
            }

            if (-1 != stderr_write_fd)
            {
                Log::get_instance()->message(ll_debug, lc_no_context, "dup2 " +
                        stringify(stderr_write_fd) + " 2");

                if (-1 == dup2(stderr_write_fd, 2))
                    throw RunCommandError("dup2 failed");

                if (-1 != stderr_close_fd)
                        close(stderr_close_fd);
            }

            Log::get_instance()->message(ll_debug, lc_no_context, "execl /bin/sh -c " + cmd);
            execl("/bin/sh", "sh", "-c", cmd.c_str(), static_cast<char *>(0));
            throw RunCommandError("execl /bin/sh -c '" + cmd + "' failed:"
                    + stringify(strerror(errno)));
        }
        else if (-1 == child)
            throw RunCommandError("fork failed: " + stringify(strerror(errno)));
        else
        {
            int status(-1);
            if (-1 == wait(&status))
                throw RunCommandError("wait failed: " + stringify(strerror(errno)));
            return ((status & 0xff00) >> 8);
        }

        throw InternalError(PALUDIS_HERE, "should never be reached");
    }
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

int
paludis::run_command(const std::string & cmd)
{
    return real_run_command(cmd, 0);
}

int
paludis::run_command_in_directory(const std::string & cmd, const FSEntry & fsentry)
{
    return real_run_command(cmd, &fsentry);
}

MakeEnvCommand::MakeEnvCommand(const std::string & c,
        const std::string & a) :
    cmd(c),
    args(a)
{
}

MakeEnvCommand
MakeEnvCommand::operator() (const std::string & k,
        const std::string & v) const
{
    std::string vv;
    for (std::string::size_type p(0) ; p < v.length() ; ++p)
        if ('\'' == v[p])
            vv.append("'\"'\"'");
        else
            vv.append(v.substr(p, 1));

    return MakeEnvCommand(cmd, args + k + "='" + vv + "' ");
}

MakeEnvCommand::operator std::string() const
{
    return "/usr/bin/env " + args + cmd;
}

const MakeEnvCommand
paludis::make_env_command(const std::string & cmd)
{
    return MakeEnvCommand(cmd, "");
}

const std::string
paludis::make_sandbox_command(const std::string & cmd)
{
#if HAVE_SANDBOX
    if (! getenv_with_default("PALUDIS_DO_NOTHING_SANDBOXY", "").empty())
    {
        Log::get_instance()->message(ll_debug, lc_no_context,
                "PALUDIS_DO_NOTHING_SANDBOXY is set, not using sandbox");
        return cmd;
    }
    else if (! getenv_with_default("SANDBOX_ACTIVE", "").empty())
    {
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Already inside sandbox, not spawning another sandbox instance");
        return cmd;
    }
    else
        return "sandbox " + cmd;
#else
    return cmd;
#endif
}

