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

#include <paludis/util/stringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>

#include <algorithm>
#include <iostream>
#include <set>
#include <cstdlib>

using namespace paludis;
using namespace cave;

using std::cout;
using std::endl;

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
            resolution_options(for_docs ? make_null_shared_ptr() : std::make_shared<ResolveCommandLineResolutionOptions>(this)),
            execution_options(for_docs ? make_null_shared_ptr() : std::make_shared<ResolveCommandLineExecutionOptions>(this)),
            display_options(for_docs ? make_null_shared_ptr() : std::make_shared<ResolveCommandLineDisplayOptions>(this)),
            graph_jobs_options(for_docs ? make_null_shared_ptr() : std::make_shared<ResolveCommandLineGraphJobsOptions>(this)),
            program_options(for_docs ? make_null_shared_ptr() : std::make_shared<ResolveCommandLineProgramOptions>(this))
        {
            add_usage_line("[ -x|--execute ] [ --uninstalls-may-break */* ] [ --remove-if-dependent */* ] spec ...");
            add_note("All options available for 'cave resolve' are also permitted. See 'man cave-resolve' for details.");
        }

        std::string app_name() const
        {
            return "cave uninstall";
        }

        std::string app_synopsis() const
        {
            return "Uninstall one or more packages.";
        }

        std::string app_description() const
        {
            return "Uninstalls one or more packages. Note that 'cave uninstall' simply rewrites the supplied "
                "dependency specifications and then uses 'cave resolve' to do the work; 'cave uninstall foo' is "
                "the same as 'cave resolve !foo'.";
        }
    };

    bool has_multiple_versions(const std::shared_ptr<const PackageIDSequence> & ids)
    {
        QualifiedPackageName old_qpn("x/x");
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if ((*i)->name() == old_qpn)
                return true;
            old_qpn = (*i)->name();
        }

        return false;
    }
}

bool
UninstallCommand::important() const
{
    return true;
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

    std::shared_ptr<Sequence<std::pair<std::string, std::string> > > targets(std::make_shared<Sequence<std::pair<std::string, std::string> >>());
    std::shared_ptr<Sequence<std::string> > targets_cleaned_up(std::make_shared<Sequence<std::string>>());

    std::set<QualifiedPackageName> qpns_being_changed;
    std::shared_ptr<PackageIDSequence> ids_going_away(std::make_shared<PackageIDSequence>());

    for (UninstallCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
            p != p_end ; ++p)
    {
        PackageDepSpec spec(parse_user_package_dep_spec(*p, env.get(), { updso_allow_wildcards }));
        const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::Matches(spec, make_null_shared_ptr(), { }) | filter::SupportsAction<UninstallAction>())]);
        if (ids->empty())
            nothing_matching_error(env.get(), *p, filter::SupportsAction<UninstallAction>());
        else if ((! cmdline.a_all_versions.specified()) && has_multiple_versions(ids))
            throw BeMoreSpecific(spec, ids, "Consider using '--" + cmdline.a_all_versions.long_name() + "'");
        else
        {
            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                qpns_being_changed.insert((*i)->name());
                std::string target("!" + stringify((*i)->name()));
                if ((*i)->slot_key())
                    target.append(":" + stringify((*i)->slot_key()->value()));
                targets->push_back(std::make_pair(target, ""));
            }

            std::copy(ids->begin(), ids->end(), ids_going_away->back_inserter());
        }
    }

    for (std::set<QualifiedPackageName>::const_iterator q(qpns_being_changed.begin()), q_end(qpns_being_changed.end()) ;
            q != q_end ; ++q)
    {
        bool removing_all_slots(true);
        const std::shared_ptr<const PackageIDSequence> all_uninstallable((*env)[selection::AllVersionsSorted(
                    generator::Package(*q) | filter::SupportsAction<UninstallAction>())]);
        for (PackageIDSequence::ConstIterator i(all_uninstallable->begin()), i_end(all_uninstallable->end()) ;
                i != i_end ; ++i)
            if (indirect_iterator(ids_going_away->end()) == std::find(
                        indirect_iterator(ids_going_away->begin()), indirect_iterator(ids_going_away->end()), **i))
            {
                removing_all_slots = false;
                break;
            }

        if (removing_all_slots)
            targets_cleaned_up->push_back("!" + stringify(*q));
        else
        {
            for (Sequence<std::pair<std::string, std::string> >::ConstIterator t(targets->begin()), t_end(targets->end()) ;
                    t != t_end ; ++t)
                targets_cleaned_up->push_back(t->first);
        }
    }

    return resolve_common(env, *cmdline.resolution_options, *cmdline.execution_options, *cmdline.display_options,
            *cmdline.graph_jobs_options, *cmdline.program_options, make_null_shared_ptr(), targets, targets_cleaned_up, false);
}

std::shared_ptr<args::ArgsHandler>
UninstallCommand::make_doc_cmdline()
{
    return std::make_shared<UninstallCommandLine>(true);
}

