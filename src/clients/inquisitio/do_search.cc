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

#include "do_search.hh"
#include "command_line.hh"
#include "matcher.hh"
#include "extractor.hh"
#include "query_task.hh"

#include <paludis/environment.hh>
#include <paludis/query.hh>
#include <paludis/package_database.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <list>
#include <set>
#include <iostream>
#include <algorithm>

using namespace paludis;
using namespace inquisitio;

int
do_search(const Environment & env)
{
    std::list<tr1::shared_ptr<Matcher> > matchers;
    std::list<tr1::shared_ptr<Extractor> > extractors;

    for (CommandLine::ParametersIterator p(CommandLine::get_instance()->begin_parameters()),
            p_end(CommandLine::get_instance()->end_parameters()) ; p != p_end ; ++p)
        matchers.push_back(MatcherMaker::get_instance()->find_maker(
                CommandLine::get_instance()->a_matcher.argument())(*p));

    for (paludis::args::StringSetArg::Iterator p(CommandLine::get_instance()->a_extractors.begin_args()),
            p_end(CommandLine::get_instance()->a_extractors.end_args()) ; p != p_end ; ++p)
        extractors.push_back(ExtractorMaker::get_instance()->find_maker(*p)(env));

    if (extractors.empty())
        extractors.push_back(ExtractorMaker::get_instance()->find_maker("name")(env));

    std::set<QualifiedPackageName> pkgs;

    for (IndirectIterator<PackageDatabase::RepositoryIterator, const Repository>
            r(env.package_database()->begin_repositories()), r_end(env.package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.specified())
            if (CommandLine::get_instance()->a_repository.end_args() == std::find(
                        CommandLine::get_instance()->a_repository.begin_args(),
                        CommandLine::get_instance()->a_repository.end_args(),
                        stringify(r->name())))
                continue;
        if (CommandLine::get_instance()->a_repository_format.specified())
            if (CommandLine::get_instance()->a_repository_format.end_args() == std::find(
                        CommandLine::get_instance()->a_repository_format.begin_args(),
                        CommandLine::get_instance()->a_repository_format.end_args(),
                        r->format()))
                continue;

        tr1::shared_ptr<const CategoryNamePartSet> cat_names(r->category_names());
        for (CategoryNamePartSet::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
        {
            if (CommandLine::get_instance()->a_category.specified())
                if (CommandLine::get_instance()->a_category.end_args() == std::find(
                            CommandLine::get_instance()->a_category.begin_args(),
                            CommandLine::get_instance()->a_category.end_args(),
                            stringify(*c)))
                    continue;

            tr1::shared_ptr<const QualifiedPackageNameSet> pkg_names(r->package_names(*c));
            for (QualifiedPackageNameSet::Iterator p(pkg_names->begin()), p_end(pkg_names->end()) ;
                    p != p_end ; ++p)
                pkgs.insert(*p);
        }
    }

    for (std::set<QualifiedPackageName>::const_iterator p(pkgs.begin()), p_end(pkgs.end()) ;
            p != p_end ; ++p)
    {
        tr1::shared_ptr<const PackageIDSequence>
            entries(env.package_database()->query(
                        query::Package(*p), qo_order_by_version)),
            preferred_entries(env.package_database()->query(query::Package(*p) &
                        query::InstalledAtRoot(env.root()), qo_order_by_version));

        if (entries->empty())
            continue;
        if (preferred_entries->empty())
            preferred_entries = entries;

        tr1::shared_ptr<const PackageID> display_entry(*preferred_entries->last());
        for (PackageIDSequence::Iterator i(preferred_entries->begin()),
                i_end(preferred_entries->end()) ; i != i_end ; ++i)
            if (! (*i)->masked())
                display_entry = *i;

        bool match(false);
        for (std::list<tr1::shared_ptr<Extractor> >::const_iterator x(extractors.begin()),
                x_end(extractors.end()) ; x != x_end && ! match ; ++x)
        {
            std::string xx((**x)(*display_entry));
            for (std::list<tr1::shared_ptr<Matcher> >::const_iterator m(matchers.begin()),
                    m_end(matchers.end()) ; m != m_end && ! match ; ++m)
                if ((**m)(xx))
                    match = true;
        }

        if (! match)
            continue;

        InquisitioQueryTask query(&env);
        query.show(PackageDepSpec(
                    tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(display_entry->name()))),
                display_entry);
    }

    return 0;
}

