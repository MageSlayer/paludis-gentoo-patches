/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include "repo_name.hh"
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/repository.hh>
#include <paludis/qa.hh>
#include <fstream>

using namespace paludis;
using namespace paludis::erepository;

bool
paludis::erepository::repo_name_check(
        QAReporter & reporter,
        const FSEntry & dir,
        const std::string & name)
{
    if (! (dir / "profiles" / "repo_name").exists())
        reporter.message(QAMessage(dir / "profiles" / "repo_name", qaml_normal, name, "No 'profiles/repo_name' file"));
    else
    {
        std::ifstream f(stringify(dir / "profiles" / "repo_name").c_str());
        if (! f)
            reporter.message(QAMessage(dir / "profiles" / "repo_name", qaml_normal, name, "repo_name file unreadable"));
        else
        {
            std::string line;
            if (! std::getline(f, line))
                reporter.message(QAMessage(dir / "profiles" / "repo_name", qaml_normal, name, "repo_name file empty"));
            else
            {
                try
                {
                    RepositoryName n(line);
                }
                catch (const RepositoryNameError &)
                {
                    reporter.message(QAMessage(dir / "profiles" / "repo_name", qaml_normal, name,
                                "repo_name not a valid repository name"));
                }

                if (std::getline(f, line))
                    reporter.message(QAMessage(dir / "profiles" / "repo_name", qaml_normal, name, "repo_name has trailing content"));
            }
        }
    }

    return true;
}

