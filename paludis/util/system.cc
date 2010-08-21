/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/stringify.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/pipe.hh>
#include <paludis/util/pty.hh>

#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>
#include "config.h"

using namespace paludis;

namespace
{
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


    static pid_t paludis_pid PALUDIS_ATTRIBUTE((used)) = get_paludis_pid();
}

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

std::string
paludis::get_user_name(const uid_t u)
{
    const struct passwd * const p(getpwuid(u));
    if (p)
        return stringify(p->pw_name);
    else
    {
        Context c("When getting user name for uid '" + stringify(u) + "':");
        Log::get_instance()->message("util.system.getpwuid", ll_warning, lc_context) <<
            "getpwuid(" << u << ") returned null";
        return stringify(u);
    }
}

std::string
paludis::get_group_name(const gid_t u)
{
    const struct group * const p(getgrgid(u));
    if (p)
        return stringify(p->gr_name);
    else
    {
        Context c("When getting group name for gid '" + stringify(u) + "':");
        Log::get_instance()->message("util.system.getgrgid", ll_warning, lc_context) <<
            "getgrgid(" << u << + ") returned null";
        return stringify(u);
    }
}

