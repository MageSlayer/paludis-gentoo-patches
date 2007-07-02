/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/e/qa/stray_files.hh>
#include <paludis/qa.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;
using namespace paludis::erepository;

bool
paludis::erepository::stray_files_check(
        QAReporter & reporter,
        const tr1::shared_ptr<const ERepository> & repo,
        const FSEntry & dir,
        const tr1::function<bool (const tr1::shared_ptr<const ERepository> &, const FSEntry &)> & stray,
        const std::string & name
        )
{
    for (DirIterator d(dir), d_end ; d != d_end ; ++d)
        if (stray(repo, *d))
            reporter.message(qaml_normal, name, "Stray file '" + stringify(*d) + "'");

    return true;
}

bool
paludis::erepository::is_stray_at_tree_dir(
        const tr1::shared_ptr<const ERepository> &,
        const FSEntry & d)
{
    if (d.is_directory_or_symlink_to_directory())
    {
        return false;
    }
    else if (d.is_regular_file_or_symlink_to_regular_file())
    {
        return true;
    }
    else
        return true;
}

