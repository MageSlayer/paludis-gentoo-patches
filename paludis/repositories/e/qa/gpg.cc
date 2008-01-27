/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Fernando J. Pereda
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

#include "gpg.hh"
#include <paludis/qa.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fd_holder.hh>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace paludis;
using namespace paludis::erepository;

bool
paludis::erepository::gpg_check(
        QAReporter & reporter,
        const FSEntry & dir,
        const std::string & name
        )
{
    Context context("When performing check '" + name + "' using gpg_check on directory '" + stringify(dir) + "':");
    Log::get_instance()->message(ll_debug, lc_context) << "gpg_check '"
        << dir << "', " << name << "'";

    FSEntry manifest(dir / "Manifest");

    if (! manifest.is_regular_file())
    {
        reporter.message(QAMessage(manifest, qaml_normal, name, "Manifest is missing or not a regular file"));
        return true;
    }

    bool is_signed(false);
    {
        std::ifstream ff(stringify(manifest).c_str());
        if (! ff)
        {
            reporter.message(QAMessage(manifest, qaml_normal, name, "Can't read Manifest file"));
            return true;
        }

        std::string s;
        if (std::getline(ff, s))
            is_signed = (0 == s.compare("-----BEGIN PGP SIGNED MESSAGE-----"));
    }

    if (is_signed)
    {
        int status(run_command("gpg --verify " + stringify(manifest) + " >/dev/null 2>/dev/null"));

        if (1 == status)
            reporter.message(QAMessage(manifest, qaml_normal, name, "Broken Manifest signature"));
        else if (2 == status)
            reporter.message(QAMessage(manifest, qaml_maybe, name, "Manifest signature cannot be verified"));
    }
    else
        reporter.message(QAMessage(manifest, qaml_minor, name, "Manifest not signed"));

    return true;
}

