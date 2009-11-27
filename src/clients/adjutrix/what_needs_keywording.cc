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

#include "what_needs_keywording.hh"
#include "command_line.hh"
#include <output/colour.hh>

#include <paludis/util/tokeniser.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/dep_list_exceptions.hh>
#include <paludis/dep_list.hh>
#include <paludis/override_functions.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/fuzzy_finder.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>
#include <tr1/functional>
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
    using namespace std::tr1::placeholders;

    int return_code(0);

    Context context("When performing what-needs-keywording action:");

    if (env.default_destinations()->empty())
    {
        std::tr1::shared_ptr<Repository> fake_destination(new FakeInstalledRepository(
                    make_named_values<FakeInstalledRepositoryParams>(
                        value_for<n::environment>(&env),
                        value_for<n::name>(RepositoryName("fake_destination")),
                        value_for<n::suitable_destination>(true),
                        value_for<n::supports_uninstall>(true)
                        )));
        env.package_database()->add_repository(1, fake_destination);
    }

    KeywordName target_keyword(*CommandLine::get_instance()->begin_parameters());
    std::string target_arch(strip_leading_string(*CommandLine::get_instance()->begin_parameters(), "~"));

    DepListOptions d_options;
    d_options.circular() = dl_circular_discard_silently;
    d_options.use() = dl_use_deps_take_all;
    d_options.blocks() = dl_blocks_discard_completely;
    d_options.override_masks().reset(new DepListOverrideMasksFunctions);
    d_options.override_masks()->push_back(std::tr1::bind(&override_tilde_keywords, &env, _1, _2));
    d_options.override_masks()->push_back(std::tr1::bind(&override_unkeyworded, &env, _1, _2));
    d_options.override_masks()->push_back(std::tr1::bind(&override_repository_masks, _2));
    d_options.match_package_options() += mpo_ignore_additional_requirements;

    DepList d(&env, d_options);

    for (CommandLine::ParametersConstIterator p(next(CommandLine::get_instance()->begin_parameters())),
            p_end(CommandLine::get_instance()->end_parameters()) ; p != p_end ; ++p)
    {
        try
        {
            d.add(parse_user_package_dep_spec(*p, &env, UserPackageDepSpecOptions()), env.default_destinations());
        }
        catch (const NoSuchPackageError & e)
        {
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << "Could not find '" << e.name() << "'.";

            if (! CommandLine::get_instance()->a_no_suggestions.specified())
            {
                cerr << " Looking for suggestions:" << endl;

                FuzzyCandidatesFinder f(env, e.name(), filter::All());

                if (f.begin() == f.end())
                    cerr << "No suggestions found." << endl;
                else
                    cerr << "Suggestions:" << endl;

                for (FuzzyCandidatesFinder::CandidatesConstIterator c(f.begin()),
                         c_end(f.end()) ; c != c_end ; ++c)
                    cerr << "  * " << colour(cl_package_name, *c) << endl;
            }

            cerr << endl;
            return 5;
        }
    }

    cout << std::setw(30) << std::left << "Package";
    cout << std::setw(20) << std::left << "Version";
    cout << std::setw(18) << std::left << "Current Keywords";
    cout << std::setw(10) << std::left << "Masks";
    cout << endl;

    cout << std::string(29, '=') << " " << std::string(19, '=') << " "
        << std::string(17, '=') << " " << std::string(9, '=') << endl;

    bool none(true);
    for (DepList::ConstIterator p(d.begin()), p_end(d.end()) ; p != p_end ; ++p)
        if (dlk_masked == p->kind())
        {
            none = false;
            cout << std::setw(30) << std::left << stringify(p->package_id()->name());
            cout << std::setw(20) << std::left << stringify(p->package_id()->canonical_form(idcf_version));

            std::string current;

            if (p->package_id()->keywords_key())
            {
                std::tr1::shared_ptr<const KeywordNameSet> keywords(p->package_id()->keywords_key()->value());
                for (KeywordNameSet::ConstIterator k(keywords->begin()), k_end(keywords->end()) ;
                        k != k_end ; ++k)
                    if (*k == KeywordName("-*")
                            || *k == target_keyword
                            || k->data().substr(1) == stringify(target_arch))
                        current.append(stringify(*k) + " ");
            }

            cout << std::setw(18) << std::left << current;

            std::string masks;

            for (PackageID::MasksConstIterator m(p->package_id()->begin_masks()), m_end(p->package_id()->end_masks()) ;
                    m != m_end ; ++m)
                masks.append(stringify((*m)->key()));

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

