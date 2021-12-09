/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/make_archive_strings.hh>
#include <paludis/repositories/e/a_finder.hh>
#include <paludis/repositories/e/aa_visitor.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>

#include <set>

using namespace paludis;
using namespace paludis::erepository;

std::pair<std::string, std::string>
paludis::erepository::make_archives_strings(
        const Environment * const env,
        const std::shared_ptr<const ERepositoryID> & id)
{
    std::string archives;
    std::string all_archives;
    std::set<std::string> already_in_archives;

    std::shared_ptr<const FetchableURISpecTree> fetches;
    if (id->fetches_key())
        fetches = id->fetches_key()->parse_value();

    /* make A */
    AFinder f(env, id);
    if (fetches)
        fetches->top()->accept(f);

    for (AFinder::ConstIterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
    {
        const FetchableURIDepSpec * const spec(static_cast<const FetchableURIDepSpec *>(i->first));

        if (already_in_archives.end() == already_in_archives.find(spec->filename()))
        {
            archives.append(spec->filename());
            already_in_archives.insert(spec->filename());
        }
        archives.append(" ");
    }

    /* make AA */
    if (! id->eapi()->supported()->ebuild_environment_variables()->env_aa().empty())
    {
        AAVisitor g;
        if (fetches)
            fetches->top()->accept(g);
        std::set<std::string> already_in_all_archives;

        for (AAVisitor::ConstIterator gg(g.begin()), gg_end(g.end()) ; gg != gg_end ; ++gg)
        {
            if (already_in_all_archives.end() == already_in_all_archives.find(*gg))
            {
                all_archives.append(*gg);
                already_in_all_archives.insert(*gg);
            }
            all_archives.append(" ");
        }
    }
    else
        all_archives = "AA-not-set-for-this-EAPI";

    return std::make_pair(archives, all_archives);
}

