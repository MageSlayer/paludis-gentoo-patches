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

#include "downgrade_check.hh"
#include "command_line.hh"
#include <paludis/query.hh>
#include <paludis/package_database.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/sequence.hh>
#include <paludis/package_id.hh>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

using namespace paludis;

namespace paludis
{
#include <src/clients/adjutrix/downgrade_check-sr.hh>
}

#include <src/clients/adjutrix/downgrade_check-sr.cc>

namespace
{
    int
    build_one_list(NoConfigEnvironment & env, std::ostream & f)
    {
        tr1::shared_ptr<const PackageIDSequence> matches(
                env.package_database()->query(query::NotMasked(), qo_group_by_slot));

        QualifiedPackageName old_package("dummy/dummy");
        SlotName old_slot("dummy");
        VersionSpec best_version("0");
        for (IndirectIterator<PackageIDSequence::ConstIterator> m(matches->begin()), m_end(matches->end()) ;
                m != m_end ; ++m)
        {
            if (m->name() != old_package || m->slot() != old_slot)
            {
                f << old_package << " " << old_slot << " " << best_version << std::endl;

                old_package = m->name();
                old_slot = m->slot();
                best_version = m->version();
            }
        }

        f << old_package << " " << old_slot << " " << best_version << std::endl;

        return 0;
    }

    std::string
    make_filename(const RepositoryEInterface::ProfilesConstIterator & p, bool unstable)
    {
        std::string result;
        FSEntry f(p->path);
        while (f.basename() != "profiles" && f != FSEntry("/"))
        {
            result = f.basename() + (result.empty() ? "" : "_" + result);
            f = f.dirname();
        }

        result.append(unstable ? ".unstable.txt" : ".stable.txt");

        return result;
    }

    std::string
    make_desc(const RepositoryEInterface::ProfilesConstIterator & p, bool unstable)
    {
        std::string result;
        FSEntry f(p->path);
        while (f.basename() != "profiles" && f != FSEntry("/"))
        {
            result = f.basename() + (result.empty() ? "" : "/" + result);
            f = f.dirname();
        }

        result.append(unstable ? " (unstable)" : " (stable)");

        return result;
    }

    void
    load_list(std::map<QPNS, VersionSpec> & map, std::istream & f)
    {
        std::string s;
        while (std::getline(f, s))
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::tokenise(s, std::back_inserter(tokens));
            if (tokens.size() != 3)
                throw ConfigurationError("Bad line '" + s + "'");

            map.insert(std::make_pair(QPNS(QualifiedPackageName(tokens.at(0)),
                            SlotName(tokens.at(1))), VersionSpec(tokens.at(2))));
        }
    }

    int
    check_one_list(NoConfigEnvironment & env, std::istream & f1,
            std::istream & f2, std::multimap<QPNS, std::string> & results,
            const std::string & desc)
    {
        int exit_status(0);

        std::map<QPNS, VersionSpec> before, after;

        load_list(before, f1);
        load_list(after, f2);

        for (std::map<QPNS, VersionSpec>::const_iterator b(before.begin()), b_end(before.end()) ;
                b != b_end ; ++b)
        {
            std::map<QPNS, VersionSpec>::const_iterator a(after.find(b->first));
            if (after.end() == a)
            {
                if (! env.package_database()->query(query::Matches(PackageDepSpec(
                                    tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(b->first.name)),
                                    tr1::shared_ptr<CategoryNamePart>(),
                                    tr1::shared_ptr<PackageNamePart>(),
                                    tr1::shared_ptr<VersionRequirements>(),
                                    vr_and,
                                    tr1::shared_ptr<SlotName>(new SlotName(b->first.slot)))),
                            qo_whatever)->empty())
                {
                    results.insert(std::make_pair(b->first, stringify(b->second) + " -> nothing on " + desc));
                    exit_status |= 2;
                }
            }
            else if (b->second > a->second)
            {
                results.insert(std::make_pair(b->first, stringify(b->second) + " -> " +
                            stringify(a->second) + " on " + desc));
                exit_status |= 4;
            }
        }

        return exit_status;
    }
}

int
do_build_downgrade_check_list(NoConfigEnvironment & env)
{
    int exit_status(0);

    FSEntry output_dir(*CommandLine::get_instance()->begin_parameters());
    if (! output_dir.mkdir())
        throw ConfigurationError("Output directory already exists");

    for (RepositoryEInterface::ProfilesConstIterator
            p(env.main_repository()->e_interface->begin_profiles()),
            p_end(env.main_repository()->e_interface->end_profiles()) ; p != p_end ; ++p)
    {
        for (int i = 0 ; i < 2 ; ++i)
        {
            env.set_accept_unstable(i);
            env.main_repository()->e_interface->set_profile(p);
            std::string n(make_filename(p, i));
            std::cerr << "Generating " << n << "..." << std::endl;
            std::ofstream f(stringify(output_dir / n).c_str());
            exit_status |= build_one_list(env, f);
        }
    }

    return exit_status;
}

int
do_downgrade_check(NoConfigEnvironment & env)
{
    int exit_status(0);

    FSEntry before_dir(*CommandLine::get_instance()->begin_parameters());
    if (! before_dir.is_directory())
        throw ConfigurationError("First input directory is not a directory");

    FSEntry after_dir(*next(CommandLine::get_instance()->begin_parameters()));
    if (! after_dir.is_directory())
        throw ConfigurationError("Second input directory is not a directory");

    std::multimap<QPNS, std::string> results;

    for (RepositoryEInterface::ProfilesConstIterator
            p(env.main_repository()->e_interface->begin_profiles()),
            p_end(env.main_repository()->e_interface->end_profiles()) ; p != p_end ; ++p)
    {
        for (int i = 0 ; i < 2 ; ++i)
        {
            env.set_accept_unstable(i);
            env.main_repository()->e_interface->set_profile(p);
            std::string n(make_filename(p, i)), desc(make_desc(p, i));

            if ((before_dir / n).exists() && (after_dir / n).exists())
            {
                std::cerr << "Checking " << n << "..." << std::endl;

                std::ifstream f1(stringify(before_dir / n).c_str());
                std::ifstream f2(stringify(after_dir / n).c_str());

                exit_status |= check_one_list(env, f1, f2, results, desc);
            }
            else
                std::cerr << "Skipping " << n << "..." << std::endl;
        }
    }

    QPNS old_qpns(QualifiedPackageName("dummy/dummmy"), SlotName("dummy"));
    for (std::multimap<QPNS, std::string>::const_iterator r(results.begin()), r_end(results.end()) ;
            r != r_end ; ++r)
    {
        if (old_qpns != r->first)
        {
            std::cout << r->first.name << " :" << r->first.slot << std::endl;
            old_qpns = r->first;
        }

        std::cout << "  " << r->second << std::endl;
    }

    return exit_status;
}

