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
#include <paludis/util/strip.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
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

    if (env.default_destinations()->empty())
    {
        tr1::shared_ptr<Repository> fake_destination(new FakeInstalledRepository(&env,
                    RepositoryName("fake_destination")));
        env.package_database()->add_repository(1, fake_destination);
    }

    KeywordName target_keyword(*CommandLine::get_instance()->begin_parameters());
    UseFlagName target_arch(strip_leading_string(
                *CommandLine::get_instance()->begin_parameters(), "~"));

    env.main_repository()->portage_interface->set_profile_by_arch(target_arch);
    env.set_accept_unstable('~' == stringify(target_keyword).at(0));

    DepListOptions d_options;
    d_options.circular = dl_circular_discard_silently;
    d_options.use = dl_use_deps_take_all;
    d_options.blocks = dl_blocks_discard_completely;
    d_options.override_masks += dl_override_tilde_keywords;
    d_options.override_masks += dl_override_unkeyworded;
    d_options.override_masks += dl_override_repository_masks;
    d_options.override_masks += dl_override_profile_masks;

    DepList d(&env, d_options);

    for (CommandLine::ParametersIterator p(next(CommandLine::get_instance()->begin_parameters())),
            p_end(CommandLine::get_instance()->end_parameters()) ; p != p_end ; ++p)
    {
        if (std::string::npos == p->find('/'))
            d.add(tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                            tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(
                                    env.package_database()->fetch_unique_qualified_package_name(PackageNamePart(*p)))))),
                    env.default_destinations());
        else
            d.add(tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(*p, pds_pm_permissive)),
                    env.default_destinations());
    }

    cout << std::setw(30) << std::left << "Package";
    cout << std::setw(20) << std::left << "Version";
    cout << std::setw(18) << std::left << "Current Keywords";
    cout << std::setw(10) << std::left << "Masks";
    cout << endl;

    cout << std::string(29, '=') << " " << std::string(19, '=') << " "
        << std::string(17, '=') << " " << std::string(9, '=') << endl;

    bool none(true);
    for (DepList::Iterator p(d.begin()), p_end(d.end()) ; p != p_end ; ++p)
        if (dlk_masked == p->kind)
        {
            none = false;
            cout << std::setw(30) << std::left << stringify(p->package.name);
            cout << std::setw(20) << std::left << stringify(p->package.version);

            std::string current;

            tr1::shared_ptr<const VersionMetadata> m(env.package_database()->fetch_repository(
                        p->package.repository)->version_metadata(p->package.name,
                            p->package.version));
            if (m->ebuild_interface)
            {
                tr1::shared_ptr<const KeywordNameCollection> keywords(m->ebuild_interface->keywords());
                for (KeywordNameCollection::Iterator k(keywords->begin()), k_end(keywords->end()) ;
                        k != k_end ; ++k)
                    if (*k == KeywordName("-*")
                            || *k == target_keyword
                            || k->data().substr(1) == stringify(target_arch))
                        current.append(stringify(*k) + " ");
            }

            cout << std::setw(18) << std::left << current;

            std::string masks;

            MaskReasons r(env.mask_reasons(p->package));
            if (r[mr_repository_mask])
                    masks.append("R");
            if (r[mr_profile_mask])
                    masks.append("P");

            cout << std::setw(10) << std::left << masks;

            cout << endl;
        }

    if (none)
    {
        cerr << "The specified package is already at the target keyword level. Perhaps" << endl;
        cerr << "you need to specify a versioned target ('>=cat/pkg-1.23')." << endl;
        return 4;
    }


    return return_code;
}


