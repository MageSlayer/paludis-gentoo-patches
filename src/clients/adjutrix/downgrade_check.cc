/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/package_database.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/package_id.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/metadata_key.hh>
#include <iostream>
#include <vector>
#include <map>

using namespace paludis;

namespace
{
    std::string slot_as_string(const PackageID & id)
    {
        if (id.slot_key())
            return stringify(id.slot_key()->value());
        else
            return "(none)";
    }

    int
    build_one_list(NoConfigEnvironment & env, std::ostream & f)
    {
        std::shared_ptr<const PackageIDSequence> matches(env[selection::AllVersionsGroupedBySlot(generator::All() | filter::NotMasked())]);

        QualifiedPackageName old_package("dummy/dummy");
        std::string old_slot("dummy");
        VersionSpec best_version("0", VersionSpecOptions());
        for (IndirectIterator<PackageIDSequence::ConstIterator> m(matches->begin()), m_end(matches->end()) ;
                m != m_end ; ++m)
        {
            if (m->name() != old_package || slot_as_string(*m) != old_slot)
            {
                f << old_package << " " << old_slot << " " << best_version << std::endl;

                old_package = m->name();
                old_slot = slot_as_string(*m);
                best_version = m->version();
            }
        }

        f << old_package << " " << old_slot << " " << best_version << std::endl;

        return 0;
    }

    void
    load_list(std::map<std::pair<QualifiedPackageName, std::string>, VersionSpec> & map, std::istream & f)
    {
        std::string s;
        while (std::getline(f, s))
        {
            std::vector<std::string> tokens;
            tokenise_whitespace(s, std::back_inserter(tokens));
            if (tokens.size() != 3)
                throw ConfigurationError("Bad line '" + s + "'");

            map.insert(std::make_pair(make_pair(QualifiedPackageName(tokens.at(0)),
                            tokens.at(1)), VersionSpec(tokens.at(2), user_version_spec_options())));
        }
    }

    int
    check_one_list(NoConfigEnvironment & env, std::istream & f1,
            std::istream & f2, std::multimap<std::pair<QualifiedPackageName, std::string>, std::string> & results,
            const std::string & desc)
    {
        int exit_status(0);

        std::map<std::pair<QualifiedPackageName, std::string>, VersionSpec> before, after;

        load_list(before, f1);
        load_list(after, f2);

        for (std::map<std::pair<QualifiedPackageName, std::string>, VersionSpec>::const_iterator
                b(before.begin()), b_end(before.end()) ;
                b != b_end ; ++b)
        {
            std::map<std::pair<QualifiedPackageName, std::string>, VersionSpec>::const_iterator
                a(after.find(b->first));
            if (after.end() == a)
            {
                PartiallyMadePackageDepSpec part_spec((PartiallyMadePackageDepSpecOptions()));
                part_spec.package(b->first.first);
                if ("(none)" != b->first.second)
                    part_spec.slot_requirement(make_shared_ptr(new UserSlotExactRequirement(SlotName(b->first.second))));
                if (! env[selection::SomeArbitraryVersion(generator::Matches(part_spec, MatchPackageOptions()))]->empty())
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

    for (int i = 0 ; i < 2 ; ++i)
    {
        bool b(i);
        env.set_accept_unstable(b);
        std::cerr << "Generating " << (b ? "unstable" : "stable")  << "..." << std::endl;
        SafeOFStream f(output_dir / ((b ? "unstable" : "stable") + std::string(".txt")));
        exit_status |= build_one_list(env, f);
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

    std::multimap<std::pair<QualifiedPackageName, std::string>, std::string> results;

    for (int i = 0 ; i < 2 ; ++i)
    {
        bool b(i);
        env.set_accept_unstable(b);
        std::string f(b ? "unstable" : "stable");
        f.append(".txt");

        if ((before_dir / f).exists() && (after_dir / f).exists())
        {
            std::cerr << "Checking " << (b ? "unstable" : "stable") << "..." << std::endl;

            SafeIFStream f1(before_dir / f);
            SafeIFStream f2(after_dir / f);

            exit_status |= check_one_list(env, f1, f2, results, b ? "unstable" : "stable");
        }
        else
            std::cerr << "Skipping " << (b ? "unstable" : "stable") << "..." << std::endl;
    }

    std::pair<QualifiedPackageName, std::string> old_qpns(QualifiedPackageName("dummy/dummmy"),
            std::string("dummy"));
    for (std::multimap<std::pair<QualifiedPackageName, std::string>, std::string>::const_iterator
            r(results.begin()), r_end(results.end()) ;
            r != r_end ; ++r)
    {
        if (old_qpns != r->first)
        {
            std::cout << r->first.first << " :" << r->first.second << std::endl;
            old_qpns = r->first;
        }

        std::cout << "  " << r->second << std::endl;
    }

    return exit_status;
}

