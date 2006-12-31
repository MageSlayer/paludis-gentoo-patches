/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/qa/repo_name_check.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/config_file.hh>

#include <set>

using namespace paludis;
using namespace paludis::qa;

RepoNameCheck::RepoNameCheck()
{
}

CheckResult
RepoNameCheck::operator() (const FSEntry & d) const
{
    CheckResult result(stringify(d), identifier());

    try
    {
        FSEntry cats(d / "repo_name");
        if (! cats.is_regular_file())
            result << Message(qal_major, "Repository name file '" + stringify(cats) + "' is not a regular file");
        else
        {
            LineConfigFile cf(cats);
            if (cf.begin() == cf.end())
                result << Message(qal_major, "Repository name file '" + stringify(cats) + "' is empty");
            else
            {
                std::string n(*cf.begin());
                try
                {
                    RepositoryName nn(n);

                    if (next(cf.begin()) != cf.end())
                        result << Message(qal_major, "Repository name file '" + stringify(cats) +
                                "' contains more than one entry");
                }
                catch (const NameError &)
                {
                    result << Message(qal_major, "Repository name file '" + stringify(cats) + "' entry '"
                            + n + "' is not a valid repository name");
                }
            }
        }
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
RepoNameCheck::identifier()
{
    static const std::string id("repo_name");
    return id;
}



