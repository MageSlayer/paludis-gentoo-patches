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

#include <paludis/repositories/e/qa/files_dir_size.hh>
#include <paludis/qa.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>

using namespace paludis;
using namespace paludis::erepository;

bool
paludis::erepository::files_dir_size_check(
        QAReporter & reporter,
        const FSEntry & dir,
        const std::string & name
        )
{
    Context context("When performing check '" + name + "' using files_dir_size_check on directory '" + stringify(dir) + "':");
    Log::get_instance()->message(ll_debug, lc_context) << "files_dir_size_check '"
        << dir << "', " << name << "'";

    struct SizeFinder
    {
        off_t total_size;

        SizeFinder() :
            total_size(0)
        {
        }

        void operator() (const FSEntry & f)
        {
            if (f.basename() == "CVS" || '.' == f.basename().at(0))
                return;

            if (f.is_directory())
            {
                for (DirIterator ff(f), ff_end ; ff != ff_end ; ++ff)
                    operator() (*ff);
            }
            else if (f.is_regular_file())
            {
                if (0 != f.basename().compare(0, 7, "digest-"))
                    total_size += f.file_size();
            }
        }
    };

    SizeFinder f;
    f(dir / "files");

    if (f.total_size > (100 * 1024))
        reporter.message(QAMessage(dir / "files", qaml_minor, name, "files/ is way too bloated (" +
                stringify(f.total_size / 1024) + "KBytes, excluding digests and CVS)"));
    else if (f.total_size > (20 * 1024))
        reporter.message(QAMessage(dir / "files", qaml_minor, name, "files/ is oversized (" +
                stringify(f.total_size / 1024) + "KBytes, excluding digests and CVS)"));

    return true;
}

