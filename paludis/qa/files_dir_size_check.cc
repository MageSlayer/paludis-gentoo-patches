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

#include <paludis/qa/files_dir_size_check.hh>

#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;
using namespace paludis::qa;

FilesDirSizeCheck::FilesDirSizeCheck()
{
}

CheckResult
FilesDirSizeCheck::operator() (const FSEntry & d) const
{
    CheckResult result(d, identifier());

    if (! (d / "files").exists())
        result << Message(qal_skip, "No files/ found");

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
    f(d / "files");

    if (f.total_size > (100 * 1024))
        result << Message(qal_minor, "files/ is way too bloated (" +
                stringify(f.total_size / 1024) + "KBytes, excluding digests and CVS)");
    else if (f.total_size > (20 * 1024))
        result << Message(qal_minor, "files/ is oversized (" +
                stringify(f.total_size / 1024) + "KBytes, excluding digests and CVS)");


    return result;
}

const std::string &
FilesDirSizeCheck::identifier()
{
    static const std::string id("files_dir_size");
    return id;
}

