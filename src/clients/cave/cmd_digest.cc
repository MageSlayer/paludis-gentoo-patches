/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include "cmd_digest.hh"
#include "exceptions.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>

#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>

#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/action.hh>
#include <paludis/partially_made_package_dep_spec.hh>

#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct DigestCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave digest";
        }

        std::string app_synopsis() const override
        {
            return "Generates a digest file for a particular package in a particular repository.";
        }

        std::string app_description() const override
        {
            return "Generates a digest file for a particular package in a particular repository.";
        }

        DigestCommandLine()
        {
            add_usage_line("cat/pkg repository");
        }
    };

    std::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return std::make_shared<StandardOutputManager>();
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }
}

int
DigestCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    DigestCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_DIGEST_OPTIONS", "CAVE_DIGEST_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (2 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("digest takes exactly two parameters");

    RepositoryName repo(*next(cmdline.begin_parameters()));
    Filter repo_filter(filter::Matches(make_package_dep_spec({ }).in_repository(repo), nullptr, { }));
    QualifiedPackageName pkg(std::string::npos == cmdline.begin_parameters()->find('/') ?
            env->fetch_unique_qualified_package_name(PackageNamePart(*cmdline.begin_parameters()), repo_filter) :
            QualifiedPackageName(*cmdline.begin_parameters()));

    auto ids((*env)[selection::AllVersionsSorted(generator::Package(pkg) & generator::InRepository(repo))]);

    if (ids->empty())
        nothing_matching_error(env.get(), *cmdline.begin_parameters(), repo_filter);

    for (const auto & i : *ids)
    {
        Context i_context("When fetching ID '" + stringify(*i) + "':");

        FetchAction a(make_named_values<FetchActionOptions>(
                    n::errors() = std::make_shared<Sequence<FetchActionFailure>>(),
                    n::exclude_unmirrorable() = false,
                    n::fetch_parts() = FetchParts() + fp_regulars + fp_extras + fp_unneeded,
                    n::ignore_not_in_manifest() = true,
                    n::ignore_unfetched() = false,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::safe_resume() = true,
                    n::want_phase() = &want_all_phases
                    ));

        if (i->supports_action(SupportsActionTest<FetchAction>()))
        {
            cout << "Fetching " << *i << "..." << endl;
            i->perform_action(a);
        }
        else
            cout << "No fetching supported for " << *i << endl;

        cout << endl;
    }

    auto r(env->fetch_repository(repo));
    if (r->manifest_interface())
    {
        cout << "Making manifest..." << endl;
        r->manifest_interface()->make_manifest(pkg);
    }
    else
    {
        cout << "Cannot make manifests for " << repo << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
DigestCommand::make_doc_cmdline()
{
    return std::make_shared<DigestCommandLine>();
}

CommandImportance
DigestCommand::importance() const
{
    return ci_development;
}

