/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh
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

#include <paludis/repositories/e/qa/misc_files.hh>
#include <paludis/qa.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>

using namespace paludis;
using namespace paludis::erepository;

bool
paludis::erepository::misc_files_check(
        QAReporter & reporter,
        const FSEntry & dir,
        const std::string & name
        )
{
    Context context("When performing check '" + name + "' using misc_files_check on directory '" + stringify(dir) + "':");
    Log::get_instance()->message(ll_debug, lc_context) << "misc_files_check '"
        << dir << "', " << name << "'";

    if (! (dir / "metadata.xml").is_regular_file())
        reporter.message(QAMessage(dir / "metadata.xml", qaml_normal, name, "metadata.xml is missing or not a regular file"));

    if ((dir / "files").exists() && ! (dir / "files").is_directory())
        reporter.message(QAMessage(dir / "files", qaml_normal, name, "files/ exists but is not a directory"));

    return true;
}

