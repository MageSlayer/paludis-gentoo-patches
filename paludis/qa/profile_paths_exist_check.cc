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

#include <paludis/qa/profile_paths_exist_check.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/config_file.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <set>

using namespace paludis;
using namespace paludis::qa;

ProfilePathsExistsCheck::ProfilePathsExistsCheck()
{
}

namespace
{
    void check_dir_recursively(const FSEntry & dir, CheckResult & result)
    {
        if (! dir.is_directory())
        {
            result << Message(qal_major, "Profile component '" + stringify(dir) + "' is not a directory");
            return;
        }

        if ((dir / "parent").exists())
        {
            LineConfigFile parent(dir / "parent", LineConfigFileOptions() + lcfo_disallow_continuations + lcfo_disallow_comments);
            for (LineConfigFile::Iterator line(parent.begin()), line_end(parent.end()) ;
                    line != line_end ; ++line)
            {
                if (stringify(*line).at(0) == '/')
                    result << Message(qal_major, "Profile component '" + stringify(dir) + "' is an absolute path");
                else
                    check_dir_recursively(dir / *line, result);
            }
        }
    }
}

CheckResult
ProfilePathsExistsCheck::operator() (const ProfileCheckData & d) const
{
    CheckResult result(stringify(d.profiles_desc_line.arch) + " " + stringify(d.profiles_desc_line.path) + " " +
            stringify(d.profiles_desc_line.status), identifier());

    try
    {
        FSEntry dir(d.profiles_desc_line.path);
        check_dir_recursively(dir, result);
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & err)
    {
        result << Message(qal_fatal, "Caught Exception '" + err.message() + "' ("
                + err.what() + ")");
    }

    return result;
}

const std::string &
ProfilePathsExistsCheck::identifier()
{
    static const std::string id("profile_paths_exist");
    return id;
}

