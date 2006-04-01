/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace paludis;

GetenvError::GetenvError(const std::string & key) throw () :
    Exception("Environment variable '" + key + "' not set")
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
    pid_t child(fork());
    int status(-1);
    if (0 == child)
    {
        Log::get_instance()->message(ll_debug, "execl /bin/sh -c " + cmd);
        execl("/bin/sh", "sh", "-c", cmd.c_str(), static_cast<char *>(0));
        throw InternalError(PALUDIS_HERE, "execl failed"); /// \todo fixme
    }
    else if (-1 == child)
    {
        throw InternalError(PALUDIS_HERE, "fork failed"); /// \todo fixme
    }
    else
    {
        if (-1 == wait(&status))
            throw InternalError(PALUDIS_HERE, "wait failed"); /// \todo fixme
    }
    return status;
}

system_internals::MakeEnvCommand::MakeEnvCommand(const std::string & c,
        const std::string & a) :
    cmd(c),
    args(a)
{
}

system_internals::MakeEnvCommand
system_internals::MakeEnvCommand::operator() (const std::string & k,
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

system_internals::MakeEnvCommand::operator std::string() const
{
    return "/usr/bin/env " + args + cmd;
}

const system_internals::MakeEnvCommand
paludis::make_env_command(const std::string & cmd)
{
    return system_internals::MakeEnvCommand(cmd, "");
}

