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

#include "cmd_uninstall.hh"
#include "resolve_cmdline.hh"
#include "resolve_common.hh"
#include "exceptions.hh"
#include "parse_spec_with_nice_error.hh"

#include <paludis/util/stringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/slot.hh>

#include <algorithm>
#include <iostream>
#include <set>
#include <cstdlib>

using namespace paludis;
using namespace cave;

using std::cout;

namespace
{
    struct UninstallCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_target_options;
        args::SwitchArg a_all_versions;

        std::shared_ptr<ResolveCommandLineResolutionOptions> resolution_options;
        std::shared_ptr<ResolveCommandLineExecutionOptions> execution_options;
        std::shared_ptr<ResolveCommandLineDisplayOptions> display_options;
        std::shared_ptr<ResolveCommandLineGraphJobsOptions> graph_jobs_options;
        std::shared_ptr<ResolveCommandLineProgramOptions> program_options;

        UninstallCommandLine(const bool for_docs) :
            g_target_options(main_options_section(), "Target options", "Target options"),
            a_all_versions(&g_target_options, "all-versions", 'a', "If a supplied spec matches multiple versions, "
                    "uninstall all versions rather than erroring", true),
            resolution_options(for_docs ? nullptr : std::make_shared<ResolveCommandLineResolutionOptions>(this)),
            execution_options(for_docs ? nullptr : std::make_shared<ResolveCommandLineExecutionOptions>(this)),
            display_options(for_docs ? nullptr : std::make_shared<ResolveCommandLineDisplayOptions>(this)),
            graph_jobs_options(for_docs ? nullptr : std::make_shared<ResolveCommandLineGraphJobsOptions>(this)),
            program_options(for_docs ? nullptr : std::make_shared<ResolveCommandLineProgramOptions>(this))
        {
            add_usage_line("[ -x|--execute ] spec ...");
            add_note("All options available for 'cave resolve' are also permitted. See 'man cave-resolve' for details.");
        }

        std::string app_name() const override
        {
            return "cave uninstall";
        }

        std::string app_synopsis() const override
        {
            return "Uninstall one or more packages.";
        }

        std::string app_description() const override
        {
            return "Uninstalls one or more packages. Note that 'cave uninstall' simply rewrites the supplied "
                "dependency specifications and then uses 'cave resolve' to do the work; 'cave uninstall foo' is "
                "the same as 'cave resolve !foo'.";
        }
    };

    bool has_multiple_versions(const std::shared_ptr<const PackageIDSequence> & ids)
    {
        QualifiedPackageName old_qpn("x/x");
        for (const auto & id : *ids)
        {
            if (id->name() == old_qpn)
                return true;
            old_qpn = id->name();
        }

        return false;
    }
}

int
UninstallCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    UninstallCommandLine cmdline(false);
    cmdline.run(args, "CAVE", "CAVE_UNINSTALL_OPTIONS", "CAVE_UNINSTALL_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    cmdline.resolution_options->apply_shortcuts();
    cmdline.resolution_options->verify(env);

    auto targets(std::make_shared<Sequence<std::pair<std::string, std::string>>>());
    auto targets_cleaned_up(std::make_shared<Sequence<std::string>>());

    std::set<QualifiedPackageName> qpns_being_changed;
    auto ids_going_away(std::make_shared<PackageIDSequence>());

    for (const auto & param : cmdline.parameters())
    {
        PackageDepSpec spec(parse_spec_with_nice_error(param, env.get(), { updso_allow_wildcards }, filter::All()));
        const std::string & cross_host = cmdline.resolution_options->a_cross_host.specified() ? cmdline.resolution_options->a_cross_host.argument() : "";
        const auto ids =
            (*env)[selection::AllVersionsSorted(generator::Matches(spec, nullptr, {}) | filter::SupportsAction<UninstallAction>() | filter::CrossCompileHost(cross_host))];

        if (ids->empty())
            nothing_matching_error(env.get(), param, filter::SupportsAction<UninstallAction>());
        else if ((! cmdline.a_all_versions.specified()) && has_multiple_versions(ids))
            throw BeMoreSpecific(spec, ids, "Consider using '--" + cmdline.a_all_versions.long_name() + "'");
        else
        {
            for (const auto & id : *ids)
            {
                qpns_being_changed.insert(id->name());
                std::string target("!" + stringify(id->name()));
                if (id->slot_key())
                    target.append(":" + stringify(id->slot_key()->parse_value().parallel_value()));
                targets->push_back(std::make_pair(target, ""));
            }

            std::copy(ids->begin(), ids->end(), ids_going_away->back_inserter());
        }
    }

    for (const auto & qpn : qpns_being_changed)
    {
        bool removing_all_slots(true);
        const auto all_uninstallable((*env)[selection::AllVersionsSorted(generator::Package(qpn) | filter::SupportsAction<UninstallAction>())]);
        for (const auto & id : *all_uninstallable)
            if (indirect_iterator(ids_going_away->end()) == std::find(indirect_iterator(ids_going_away->begin()),
                                                                      indirect_iterator(ids_going_away->end()),
                                                                      *id))
            {
                removing_all_slots = false;
                break;
            }

        if (removing_all_slots)
            targets_cleaned_up->push_back("!" + stringify(qpn));
        else
        {
            for (const auto & target : *targets)
                targets_cleaned_up->push_back(target.first);
        }
    }

    return resolve_common(env, *cmdline.resolution_options, *cmdline.execution_options, *cmdline.display_options,
            *cmdline.graph_jobs_options, *cmdline.program_options, nullptr, targets, targets_cleaned_up, false);
}

std::shared_ptr<args::ArgsHandler>
UninstallCommand::make_doc_cmdline()
{
    return std::make_shared<UninstallCommandLine>(true);
}

CommandImportance
UninstallCommand::importance() const
{
    return ci_core;
}

