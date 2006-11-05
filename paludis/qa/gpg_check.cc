/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Fernando J. Pereda <ferdy@gentoo.org>
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

#include <paludis/qa/gpg_check.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fd_holder.hh>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace paludis;
using namespace paludis::qa;

GPGCheck::GPGCheck()
{
}

CheckResult
GPGCheck::operator() (const FSEntry & d) const
{
    CheckResult result(d, identifier());
    FSEntry manifest(d / "Manifest");

    if (! manifest.is_regular_file())
    {
        result << Message(qal_major, "No Manifest");
        return result;
    }

    bool is_signed(false);
    {
        std::ifstream ff(stringify(manifest).c_str());
        if (! ff)
            result << Message(qal_major, "Can't read Manifest file");
        else
        {
            std::string s;
            if (std::getline(ff, s))
                is_signed = (0 == s.compare("-----BEGIN PGP SIGNED MESSAGE-----"));
        }
    }

    if (is_signed)
    {
        FDHolder dev_null(::open("/dev/null", O_WRONLY));

        set_run_command_stdout_fds(dev_null, -1);
        set_run_command_stderr_fds(dev_null, -1);

        int status(run_command("gpg --verify " + stringify(manifest)));

        if (1 == status)
            result << Message(qal_major, "Broken Manifest signature");
        else if (2 == status)
            result << Message(qal_maybe, "Manifest signature cannot be verified");
    }
    else
        result << Message(qal_minor, "Manifest not signed");

    return result;
}

const std::string &
GPGCheck::identifier()
{
    static const std::string id("gpg");
    return id;
}
