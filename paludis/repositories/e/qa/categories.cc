/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include "categories.hh"
#include <paludis/qa.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/layout.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/log.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <set>

using namespace paludis;

bool
paludis::erepository::categories_check(
        QAReporter & reporter,
        const std::tr1::shared_ptr<const ERepository> & repo,
        const std::string & name)
{
    Context context("When performing check '" + name + "':");
    FSEntry cats(repo->layout()->categories_file());
    Log::get_instance()->message("e.qa.categories_check", ll_debug, lc_context) << "categories_check '"
        << cats << "', " << name << "'";

    try
    {
        if (! cats.is_regular_file_or_symlink_to_regular_file())
        {
            if (cats.exists() || ! repo->params().master_repository)
                reporter.message(QAMessage(cats, qaml_severe, name, "Categories file is not a regular file"));
        }
        else
        {
            LineConfigFile cf(cats, LineConfigFileOptions());
            std::set<CategoryNamePart> c;

            for (LineConfigFile::ConstIterator line(cf.begin()), line_end(cf.end()) ;
                    line != line_end ; ++line)
            {
                try
                {
                    CategoryNamePart n(*line);
                    std::pair<std::set<CategoryNamePart>::const_iterator, bool> r(c.insert(n));

                    if (! r.second)
                        reporter.message(QAMessage(cats, qaml_normal, name, "Categories file entry '"
                                + stringify(*line) + "' is not unique"));
                    else if (next(r.first) != c.end())
                        reporter.message(QAMessage(cats, qaml_minor, name, "Categories file entry '"
                                + stringify(*line) + "' is not in order"));
                    else
                    {
                        FSEntry f(repo->layout()->category_directory(n));

                        if (! f.is_directory_or_symlink_to_directory())
                        {
                            if (f.exists())
                                reporter.message(QAMessage(cats, qaml_normal, name, "Categories file entry '"
                                        + stringify(*line) + "' exists but is not a directory"));
                            else
                                reporter.message(QAMessage(cats, qaml_minor, name, "Categories file entry '"
                                        + stringify(*line) + "' does not exist"));
                        }
                    }
                }
                catch (const NameError &)
                {
                    reporter.message(QAMessage(cats, qaml_severe, name, "Categories file entry '"
                            + stringify(*line) + "' is not a valid category name"));
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
        reporter.message(QAMessage(cats, qaml_severe, name, "Caught Exception '" + err.message() + "' ("
                + err.what() + ") when handling categories file"));
    }

    return true;
}

