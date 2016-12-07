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

#include "cmd_fix_linkage.hh"
#include "resolve_cmdline.hh"
#include "cmd_resolve_display_callback.hh"
#include "resolve_common.hh"

#include <paludis/args/do_help.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/broken_linkage_finder.hh>
#include <paludis/package_id.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_requirements.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/slot.hh>

#include <iostream>
#include <set>
#include <cstdlib>

using namespace paludis;
using namespace cave;

using std::cout;
using std::endl;

namespace
{
    struct FixLinkageCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_execution_options;
        args::SwitchArg a_execute;

        args::ArgsGroup g_linkage_options;
        args::StringSetArg a_libraries;
        args::SwitchArg a_exact;

        FixLinkageCommandLine() :
            g_execution_options(main_options_section(), "Execution Options", "Control execution."),
            a_execute(&g_execution_options, "execute", 'x', "Execute the suggested actions", true),
            g_linkage_options(main_options_section(), "Linkage options", "Options relating to linkage"),
            a_libraries(&g_linkage_options, "library", 'l', "Only rebuild packages linked against this library, even if it exists. May be specified multiple times."),
            a_exact(&g_linkage_options, "exact", 'e', "Rebuild the same package version that is currently installed", true)
        {
            add_usage_line("[ -x|--execute ] [ --library foo.so.1 ] [ -- options for 'cave resolve' ]");

            add_note("This command uses the same underlying logic as 'cave resolve'. Any option that is "
                    "valid for 'cave resolve' may be passed as a parameter following a '--'. For example, "
                    "'cave fix-linkage --library foo.so.1 cat/pkg -- --lazy' may be useful. As a "
                    "special case, '--execute' does not require a '--'.");
            add_note("By default, '--lazy' and '--preserve-world' are both provided automatically. To "
                    "override this, use '-- --no-lazy --no-preserve-world'.");
        }

        std::string app_name() const override
        {
            return "cave fix-linkage";
        }

        std::string app_synopsis() const override
        {
            return "Identify packages with broken linkage that can be fixed by rebuilds.";
        }

        std::string app_description() const override
        {
            return "Identifies packages with broken linkage that can be fixed by rebuilds. "
                "If instructed, then executes the relevant install and uninstall actions to "
                "do said fixing.";
        }
    };

    struct OptionsForResolve :
        args::ArgsHandler
    {
        ResolveCommandLineResolutionOptions resolution_options;
        ResolveCommandLineExecutionOptions execution_options;
        ResolveCommandLineDisplayOptions display_options;
        ResolveCommandLineGraphJobsOptions graph_jobs_options;
        ResolveCommandLineProgramOptions program_options;

        OptionsForResolve() :
            resolution_options(this),
            execution_options(this),
            display_options(this),
            graph_jobs_options(this),
            program_options(this)
        {
        }

        std::string app_name() const override
        {
            return "";
        }

        std::string app_synopsis() const override
        {
            return "";
        }

        std::string app_description() const override
        {
            return "";
        }
    };
}

int
FixLinkageCommand::run(const std::shared_ptr<Environment> & env,
                       const std::shared_ptr<const Sequence<std::string > > & args)
{
    FixLinkageCommandLine cmdline;
    OptionsForResolve resolve_cmdline;

    cmdline.run(args, "CAVE", "CAVE_FIX_LINKAGE_OPTIONS", "CAVE_FIX_LINKAGE_CMDLINE",
                args::ArgsHandlerOptions() + args::aho_separate_after_dashes);

    resolve_cmdline.resolution_options.a_lazy.set_specified(args::aos_weak);
    resolve_cmdline.execution_options.a_preserve_world.set_specified(args::aos_weak);
    resolve_cmdline.run(cmdline.separate_after_dashes_args(),
                        "CAVE", "CAVE_RESOLVE_OPTIONS", "CAVE_RESOLVE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("fix-linkage takes no parameters");

    resolve_cmdline.resolution_options.apply_shortcuts();
    resolve_cmdline.resolution_options.verify(env);

    if (cmdline.a_execute.specified())
        resolve_cmdline.resolution_options.a_execute.set_specified(args::aos_specified);

    auto libraries(std::make_shared<Sequence<std::string>>());
    for (const auto & library : cmdline.a_libraries.args())
    {
        libraries->push_back(library);
        if (std::string::npos != library.find('/'))
        {
            FSPath f(library);
            libraries->push_back(f.basename());
            Log::get_instance()->message("cave.fix_linkage.library_path", ll_warning, lc_no_context)
                << "Argument --" << cmdline.a_libraries.long_name() << " '" << library
                << "' includes a '/', which probably does not do what you want. Generally you should not specify a path to a library.";
        }
    }

    std::shared_ptr<BrokenLinkageFinder> finder;
    {
        DisplayCallback display_callback("Searching: ");
        ScopedNotifierCallback display_callback_holder(env.get(), NotifierCallbackFunction(std::cref(display_callback)));
        finder = std::make_shared<BrokenLinkageFinder>(env.get(), libraries);
    }

    if (finder->begin_broken_packages() == finder->end_broken_packages())
    {
        if (libraries->empty())
            cout << "No broken packages found" << endl;
        else
            cout << "No packages that depend on " << join(libraries->begin(), libraries->end(), ", ") << " found" << endl;

        return EXIT_SUCCESS;
    }

    std::shared_ptr<Sequence<std::pair<std::string, std::string>>> targets(std::make_shared<Sequence<std::pair<std::string, std::string>>>());

    for (const auto & package : finder->broken_packages())
    {
        cout << endl;

        cout << "* " << *package << endl;

        std::set<FSPath, FSPathComparator> broken_files;
        for (const auto & file : finder->broken_files(package))
        {
            cout << "    " << file;
            cout << " (requires " << join(finder->begin_missing_requirements(package, file), finder->end_missing_requirements(package, file), " ") << ")";
            std::copy(finder->begin_missing_requirements(package, file), finder->end_missing_requirements(package, file),
                      create_inserter<FSPath>(std::inserter(broken_files, broken_files.end())));
            cout << endl;
        }

        PartiallyMadePackageDepSpec part_spec({ });
        part_spec.package(package->name());
        if (package->slot_key())
            part_spec.slot_requirement(std::make_shared<UserSlotExactPartialRequirement>(package->slot_key()->parse_value().parallel_value()));

        if (cmdline.a_exact.specified())
            part_spec.version_requirement(make_named_values<VersionRequirement>(n::version_operator() = vo_equal,
                                                                                n::version_spec() = package->version()));

        targets->push_back(std::make_pair(stringify(PackageDepSpec(part_spec)), join(broken_files.begin(), broken_files.end(), ", ")));
    }

    std::shared_ptr<const PackageID> orphans;
    if (finder->begin_broken_files(orphans) != finder->end_broken_files(orphans))
    {
        if (libraries->empty())
            cout << endl << "The following broken files are not owned by any installed package:" << endl;
        else
            cout << endl << "The following files that depend on the specified library(ies) are not owned by any installed package:" << endl;

        for (const auto & file : finder->broken_files(orphans))
        {
            cout << "    " << file;
            cout << " (requires " << join(finder->begin_missing_requirements(orphans, file), finder->end_missing_requirements(orphans, file), " ") << ")";
            cout << endl;
        }
    }

    return resolve_common(env, resolve_cmdline.resolution_options, resolve_cmdline.execution_options, resolve_cmdline.display_options, resolve_cmdline.graph_jobs_options,
                          resolve_cmdline.program_options, nullptr, targets, nullptr, false);
}

std::shared_ptr<args::ArgsHandler>
FixLinkageCommand::make_doc_cmdline()
{
    return std::make_shared<FixLinkageCommandLine>();
}

CommandImportance
FixLinkageCommand::importance() const
{
    return ci_core;
}

