/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2008 Ciaran McCreesh
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

#include "changelog.hh"
#include <paludis/qa.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <fstream>

using namespace paludis;

bool
paludis::erepository::changelog_check(
        QAReporter & reporter,
        const FSEntry & dir,
        const QualifiedPackageName & qpn,
        const std::string & name
        )
{
    Context context("When performing check '" + name + "' using changelog_check on directory '" + stringify(dir) + "':");
    Log::get_instance()->message("e.qa.changelog_check", ll_debug, lc_context) << "changelog_check '"
        << dir << "', " << name << "'";

    FSEntry f(dir / "ChangeLog");

    if (! f.exists())
        reporter.message(QAMessage(f, qaml_normal, name, "No ChangeLog found"));
    else if (! f.is_regular_file_or_symlink_to_regular_file())
        reporter.message(QAMessage(f, qaml_normal, name, "Not a regular file"));
    else
    {
        std::ifstream ff(stringify(f).c_str());
        do
        {
            if (! ff)
            {
                reporter.message(QAMessage(f, qaml_normal, name, "Can't read ChangeLog"));
                continue;
            }

            std::string s;
            if (! std::getline(ff, s))
            {
                reporter.message(QAMessage(f, qaml_normal, name, "Can't read ChangeLog header"));
                continue;
            }

            if (s != "# ChangeLog for " + stringify(qpn))
                reporter.message(QAMessage(f, qaml_minor, name, "ChangeLog header is incorrect"));
        }
        while (false);
    }

    return true;
}

