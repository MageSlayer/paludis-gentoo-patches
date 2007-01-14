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

#include "what_needs_keywording.hh"
#include "command_line.hh"

#include <paludis/util/tokeniser.hh>
#include <paludis/util/compare.hh>
#include <paludis/util/strip.hh>
#include <paludis/repositories/portage/portage_repository.hh>
#include <paludis/dep_list/exceptions.hh>
#include <paludis/dep_list/dep_list.hh>

#include <set>
#include <map>
#include <list>
#include <iostream>
#include <iomanip>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

int do_what_needs_keywording(NoConfigEnvironment & env)
{
    int return_code(0);

    Context context("When performing what-needs-keywording action:");

    KeywordName target_keyword(*CommandLine::get_instance()->begin_parameters());
    UseFlagName target_arch(strip_leading_string(
                *CommandLine::get_instance()->begin_parameters(), "~"));

    env.portage_repository()->set_profile_by_arch(target_arch);
    env.set_accept_unstable('~' == stringify(target_keyword).at(0));

    DepListOptions d_options;
    d_options.circular = dl_circular_discard;
    d_options.override_masks.set(dl_override_tilde_keywords);
    d_options.override_masks.set(dl_override_unkeyworded);

    DepList d(&env, d_options);

    cout << std::setw(30) << std::left << "Package";
    cout << std::setw(20) << std::left << "Version";
    cout << std::setw(20) << std::left << "Current Keywords";
    cout << endl;

    cout << std::string(29, '=') << " " << std::string(19, '=') << " "
        << std::string(19, '=') << endl;

    for (CommandLine::ParametersIterator p(next(CommandLine::get_instance()->begin_parameters())),
            p_end(CommandLine::get_instance()->end_parameters()) ; p != p_end ; ++p)
        d.add(PackageDepAtom::Pointer(new PackageDepAtom(*p)));

    for (DepList::Iterator p(d.begin()), p_end(d.end()) ; p != p_end ; ++p)
        if (dlk_masked == p->kind)
        {
            cout << std::setw(30) << std::left << stringify(p->package.name);
            cout << std::setw(20) << std::left << stringify(p->package.version);

            VersionMetadata::ConstPointer m(env.package_database()->fetch_repository(
                        p->package.repository)->version_metadata(p->package.name,
                            p->package.version));
            if (m->get_ebuild_interface())
            {
                std::set<std::string> keywords;
                WhitespaceTokeniser::get_instance()->tokenise(m->get_ebuild_interface()->keywords,
                        std::inserter(keywords, keywords.end()));
                for (std::set<std::string>::const_iterator k(keywords.begin()), k_end(keywords.end()) ;
                        k != k_end ; ++k)
                    if (*k == "-*"
                            || *k == stringify(target_keyword)
                            || k->substr(1) == stringify(target_arch))
                        cout << *k << " ";
            }

            cout << endl;
        }

    return return_code;
}


