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

#include <paludis/qa/categories_check.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/config_file.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <set>

using namespace paludis;
using namespace paludis::qa;

CategoriesCheck::CategoriesCheck()
{
}

CheckResult
CategoriesCheck::operator() (const FSEntry & d) const
{
    CheckResult result(stringify(d), identifier());

    try
    {
        FSEntry cats(d / "categories");
        if (! cats.is_regular_file())
            result << Message(qal_fatal, "Categories file '" + stringify(cats) + "' is not a regular file");
        else
        {
            LineConfigFile cf(cats, LineConfigFileOptions());
            std::set<CategoryNamePart> c;

            for (LineConfigFile::Iterator line(cf.begin()), line_end(cf.end()) ;
                    line != line_end ; ++line)
            {
                try
                {
                    CategoryNamePart n(*line);
                    std::pair<std::set<CategoryNamePart>::const_iterator, bool> r(c.insert(n));
                    if (! r.second)
                        result << Message(qal_major, "Categories file '" + stringify(cats) + "' entry '"
                                + stringify(*line) + "' is not unique");
                    else if (next(r.first) != c.end())
                        result << Message(qal_minor, "Categories file '" + stringify(cats) + "' entry '"
                                + stringify(*line) + "' is not in order");
                    else
                    {
                        FSEntry f(d.dirname() / stringify(n));
                        if (! f.is_directory())
                        {
                            if (f.exists())
                                result << Message(qal_major, "Categories file '" + stringify(cats) + "' entry '"
                                        + stringify(*line) + "' exists but is not a directory");
                            else
                                result << Message(qal_minor, "Categories file '" + stringify(cats) + "' entry '"
                                        + stringify(*line) + "' does not exist");
                        }
                    }
                }
                catch (const NameError &)
                {
                    result << Message(qal_fatal, "Categories file '" + stringify(cats) + "' entry '"
                            + stringify(*line) + "' is not a valid category name");
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
CategoriesCheck::identifier()
{
    static const std::string id("categories");
    return id;
}


