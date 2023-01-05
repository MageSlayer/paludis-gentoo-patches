/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include "cmd_import.hh"
#include "exceptions.hh"
#include "resolve_common.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/join.hh>
#include <paludis/util/map.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/environment.hh>
#include <paludis/version_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>
#include <paludis/metadata_key.hh>
#include <paludis/repository_factory.hh>
#include <paludis/unformatted_pretty_printer.hh>

#include <iostream>
#include <cstdlib>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
    struct ImportCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave import";
        }

        std::string app_synopsis() const override
        {
            return "Import a package from a directory containing its image.";
        }

        std::string app_description() const override
        {
            return "Import a package from a directory containing its image. A named directory "
                "is treated as holding the contents to install, and a dummy package name is "
                "provided on the command line. Safe merge, unmerge, upgrade and replace support "
                "is provided, as is content tracking for installed files.";
        }

        args::ArgsGroup g_execution_options;
        args::SwitchArg a_execute;

        args::ArgsGroup g_contents_options;
        args::StringArg a_location;
        args::StringArg a_install_under;
        args::IntegerArg a_rewrite_ids_over_to_root;
        args::EnumArg a_strip;
        args::EnumArg a_preserve_work;

        args::ArgsGroup g_metadata_options;
        args::StringArg a_description;
        args::StringSetArg a_build_dependency;
        args::StringSetArg a_run_dependency;
        args::SwitchArg a_preserve_metadata;

        ImportCommandLine() :
            g_execution_options(main_options_section(), "Execution Options", "Control execution."),
            a_execute(&g_execution_options, "execute", 'x', "Execute the suggested actions", true),

            g_contents_options(main_options_section(), "Contents Options",
                    "Options controlling the content to install"),
            a_location(&g_contents_options, "location", 'l',
                    "Specify the directory containing the image to install (default: current directory)"),
            a_install_under(&g_contents_options, "install-under", 'u',
                    "Install under the specified directory, rather than /"),
            a_rewrite_ids_over_to_root(&g_contents_options, "rewrite-ids-over-to-root", 'r',
                    "Change any UID or GID over this value to 0 (-1 disables, default)"),
            a_strip(&g_contents_options, "strip", 's',
                    "Specify whether to strip and split objects being installed.",
                    args::EnumArg::EnumArgOptions
                    ("config",       'c',   "Use build_options: symbols from user configuration")
                    ("always",       'a',   "Split and strip, regardless of user configuration")
                    ("never",        'n',   "Do not split or strip, regardless of user configuration"),
                    "config"),
            a_preserve_work(&g_contents_options, "preserve-work", 'p',
                    "Specify whether to avoid carrying out destructive merge actions (e.g. using rename() rather than copying)",
                    args::EnumArg::EnumArgOptions
                    ("config",       'c',   "Use build_options: preserve_work from user configuration")
                    ("always",       'a',   "Do not merge destructively, regardless of user configuration")
                    ("never",        'n',   "Allow desructive merges, regardless of user configuration"),
                    "config"),

            g_metadata_options(main_options_section(), "Metadata Options",
                    "Options specifying metadata for the package being installed"),
            a_description(&g_metadata_options, "description", 'D',
                    "Specify a description for the package"),
            a_build_dependency(&g_metadata_options, "build-dependency", 'B',
                    "Specify a build dependency. May be specified multiple times."),
            a_run_dependency(&g_metadata_options, "run-dependency", 'R',
                    "Specify a run dependency. May be specified multiple times."),
            a_preserve_metadata(&g_metadata_options, "preserve-metadata", 'P',
                    "If replacing a package previously installed using this command, copy its description "
                    "and dependencies", true)
        {
            add_usage_line("[ --location blah ] cat/pkg [ version ] [ slot ] "
                    "[ --execute ] [ -- options for 'cave resolve' ]");

            add_note("This command uses the same underlying logic as 'cave resolve'. Any option that is "
                    "valid for 'cave resolve' may be passed as a parameter following a '--'. For example, "
                    "'cave import cat/pkg --build-dependency some/dep -- --lazy' may be useful. As a "
                    "special case, '--execute' does not require a '--'.");
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

    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }
}

int
ImportCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    ImportCommandLine cmdline;
    OptionsForResolve resolve_cmdline;

    cmdline.run(args, "CAVE", "CAVE_IMPORT_OPTIONS", "CAVE_IMPORT_CMDLINE",
            args::ArgsHandlerOptions() + args::aho_separate_after_dashes);
    resolve_cmdline.run(cmdline.separate_after_dashes_args(),
            "CAVE", "CAVE_RESOLVE_OPTIONS", "CAVE_RESOLVE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) < 1 ||
            std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) > 3)
        throw args::DoHelp("import takes between one and three parameters");

    QualifiedPackageName package(*cmdline.begin_parameters());
    VersionSpec version((next(cmdline.begin_parameters()) != cmdline.end_parameters()) ?
            *next(cmdline.begin_parameters()) : "0", user_version_spec_options());
    SlotName slot(
            (next(cmdline.begin_parameters()) != cmdline.end_parameters() &&
             next(cmdline.begin_parameters(), 2) != cmdline.end_parameters()) ?
            *next(cmdline.begin_parameters(), 2) : "0");

    std::string build_dependencies;
    std::string run_dependencies;
    std::string description;

    if (cmdline.a_preserve_metadata.specified())
    {
        std::shared_ptr<const PackageIDSequence> old_ids((*env)[selection::AllVersionsSorted(generator::Package(package))]);
        std::shared_ptr<const PackageID> old_id;
        for (PackageIDSequence::ConstIterator i(old_ids->begin()), i_end(old_ids->end()) ;
                i != i_end ; ++i)
        {
            auto repo(env->fetch_repository((*i)->repository_name()));
            if (! repo->format_key())
                continue;
            if (repo->format_key()->parse_value() != "installed_unpackaged")
                continue;
            old_id = *i;
            break;
        }

        if (! old_id)
            throw args::DoHelp("--" + cmdline.a_preserve_metadata.long_name() + " specified but "
                    "no old ID available");

        if (old_id->short_description_key())
            description = old_id->short_description_key()->parse_value();
        if (old_id->build_dependencies_key())
            build_dependencies = old_id->build_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { });
        if (old_id->run_dependencies_key())
            run_dependencies = old_id->run_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { });
    }

    if (cmdline.a_description.specified())
        description = cmdline.a_description.argument();
    if (cmdline.a_build_dependency.specified())
        build_dependencies = join(
                cmdline.a_build_dependency.begin_args(),
                cmdline.a_build_dependency.end_args(), ", ");
    if (cmdline.a_run_dependency.specified())
        run_dependencies = join(
                cmdline.a_run_dependency.begin_args(),
                cmdline.a_run_dependency.end_args(), ", ");

    std::string strip;
    if (cmdline.a_strip.argument() == "never")
        strip = "false";
    else if (cmdline.a_strip.argument() == "always")
        strip = "true";

    std::string preserve_work;
    if (cmdline.a_preserve_work.argument() == "never")
        preserve_work = "false";
    else if (cmdline.a_preserve_work.argument() == "always")
        preserve_work = "true";

    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("location", stringify(
                cmdline.a_location.specified() ?
                FSPath(cmdline.a_location.argument()) :
                FSPath::cwd()));
    keys->insert("install_under", stringify(
                cmdline.a_install_under.specified() ?
                FSPath(cmdline.a_install_under.argument()) :
                FSPath("/")));
    keys->insert("rewrite_ids_over_to_root", stringify(
                cmdline.a_rewrite_ids_over_to_root.specified() ?
                cmdline.a_rewrite_ids_over_to_root.argument() : -1));
    keys->insert("format", "unpackaged");
    keys->insert("name", stringify(package));
    keys->insert("version", stringify(version));
    keys->insert("slot", stringify(slot));
    keys->insert("description", description);
    keys->insert("build_dependencies", build_dependencies);
    keys->insert("run_dependencies", run_dependencies);
    keys->insert("strip", strip);
    keys->insert("preserve_work", preserve_work);
    std::shared_ptr<Repository> repo(RepositoryFactory::get_instance()->create(env.get(),
                std::bind(from_keys, keys, std::placeholders::_1)));
    env->add_repository(10, repo);
    std::shared_ptr<const PackageIDSequence> ids(repo->package_ids(package, { }));
    if (1 != std::distance(ids->begin(), ids->end()))
        throw InternalError(PALUDIS_HERE, "ids is '" + join(indirect_iterator(ids->begin()), indirect_iterator(
                        ids->end()), " ") + "'");

    resolve_cmdline.resolution_options.apply_shortcuts();
    resolve_cmdline.resolution_options.verify(env);

    if (cmdline.a_execute.specified())
        resolve_cmdline.resolution_options.a_execute.set_specified(args::aos_specified);

    std::shared_ptr<Sequence<std::pair<std::string, std::string> > > targets(std::make_shared<Sequence<std::pair<std::string, std::string> >>());
    targets->push_back(std::make_pair(stringify((*ids->begin())->uniquely_identifying_spec()), ""));

    std::shared_ptr<Sequence<std::string> > world_specs(std::make_shared<Sequence<std::string>>());
    world_specs->push_back(stringify((*ids->begin())->name()));

    return resolve_common(env,
            resolve_cmdline.resolution_options,
            resolve_cmdline.execution_options,
            resolve_cmdline.display_options,
            resolve_cmdline.graph_jobs_options,
            resolve_cmdline.program_options,
            keys, targets, world_specs, false);
}

std::shared_ptr<args::ArgsHandler>
ImportCommand::make_doc_cmdline()
{
    return std::make_shared<ImportCommandLine>();
}

CommandImportance
ImportCommand::importance() const
{
    return ci_core;
}

