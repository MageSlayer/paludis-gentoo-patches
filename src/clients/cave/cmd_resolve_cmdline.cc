/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#include "cmd_resolve_cmdline.hh"
#include <paludis/args/do_help.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/map.hh>
#include <paludis/repository_factory.hh>
#include <tr1/memory>

using namespace paludis;
using namespace cave;

namespace
{
    std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }
}

ResolveCommandLineResolutionOptions::ResolveCommandLineResolutionOptions(args::ArgsHandler * const h) :
    ArgsSection(h, "Resolution Options"),
    g_execution_options(this, "Execution Options", "Control execution."),
    a_execute(&g_execution_options, "execute", 'x', "Execute the suggested actions", true),

    g_convenience_options(this, "Convenience Options", "Broad behaviour options."),
    a_lazy(&g_convenience_options, "lazy", 'z', "Do as little work as possible.", true),
    a_complete(&g_convenience_options, "complete", 'c', "Do all optional work.", true),
    a_everything(&g_convenience_options, "everything", 'e', "Do all optional work, and also reinstall", true),

//            g_resolution_options(this, "Resolution Options", "Resolution options."),
//            a_permit_slot_uninstalls(&g_resolution_options, "permit-slot-uninstalls", '\0',
//                    "Uninstall slots of packages if they are blocked by other slots of that package", true),
//            a_permit_uninstalls(&g_resolution_options, "permit-uninstalls", '\0',
//                    "Uninstall packages that are blocked", true),
//            a_permit_downgrades(&g_resolution_options, "permit-downgrades", '\0', "Permit downgrades", true),
//            a_permit_unsafe_uninstalls(&g_resolution_options, "permit-unsafe-uninstalls", '\0',
//                    "Permit uninstalls even if the uninstall isn't known to be safe", true),
//
//            g_cleanup_options(this, "Cleanup Options", "Cleanup options."),
//            a_purge_unused_slots(&g_cleanup_options, "purge-unused-slots", '\0',
//                    "Purge slots that are no longer used after an uninstall or clean", true),
//            a_purge_unused_packages(&g_cleanup_options, "purge-unused-packages", '\0',
//                    "Purge packages that are no longer used after an uninstall or clean", true),
//

    g_keep_options(this, "Reinstall Options", "Control whether installed packages are kept."),
    a_keep_targets(&g_keep_options, "keep-targets", 'K',
            "Select whether to keep target packages",
            args::EnumArg::EnumArgOptions
            ("auto",                  'a', "If the target is a set, if-same, otherwise never")
            ("never",                 'n', "Never")
            ("if-transient",          't', "Only if the installed package is transient "
                                           "(e.g. from 'importare')")
            ("if-same",               's', "If it is the same as the proposed replacement")
            ("if-same-version",       'v', "If it is the same version as the proposed replacement")
            ("if-possible",           'p', "If possible"),

            "auto"
            ),
    a_keep(&g_keep_options, "keep", 'k',
            "Select whether to keep installed packages that are not targets",
            args::EnumArg::EnumArgOptions
            ("never",                 'n', "Never")
            ("if-transient",          't', "Only if the installed package is transient "
                                           "(e.g. from 'importare') (default if --everything)")
            ("if-same",               's', "If it is the same as the proposed replacement "
                                           "(default if --complete)")
            ("if-same-version",       'v', "If it is the same version as the proposed replacement")
            ("if-possible",           'p', "If possible"),

            "if-possible"
            ),
    a_reinstall_scm(&g_keep_options, "reinstall-scm", 'R',
            "Select whether to reinstall SCM packages that would otherwise be kept",
            args::EnumArg::EnumArgOptions
            ("always",                'a', "Always")
            ("daily",                 'd', "If they were installed more than a day ago")
            ("weekly",                'w', "If they were installed more than a week ago (default if --complete)")
            ("never",                 'n', "Never"),
            "never"
            ),
//            a_reinstall_for_removals(&g_reinstall_options, "reinstall-for-removals", '\0',
//                    "Select whether to rebuild packages if rebuilding would avoid an unsafe removal", true),
//
    g_slot_options(this, "Slot Options", "Control which slots are considered."),
    a_target_slots(&g_slot_options, "target-slots", 'S',
            "Which slots to consider for targets",
            args::EnumArg::EnumArgOptions
            ("best-or-installed",     'x', "Consider the best slot, if it is not installed, "
                                           "or all installed slots otherwise")
            ("installed-or-best",     'i', "Consider all installed slots, or the best "
                                           "installable slot if nothing is installed")
            ("all",                   'a', "Consider all installed slots and the best installable slot"
                                           " (default if --complete or --everything)")
            ("best",                  'b', "Consider the best installable slot only"
                                           " (default if --lazy)"),
            "best-or-installed"
            ),
    a_slots(&g_slot_options, "slots", 's',
            "Which slots to consider for packages that are not targets",
            args::EnumArg::EnumArgOptions
            ("best-or-installed",     'x', "Consider the best slot, if it is not installed, "
                                           "or all installed slots otherwise")
            ("installed-or-best",     'i', "Consider all installed slots, or the best "
                                           "installable slot if nothing is installed")
            ("all",                   'a', "Consider all installed slots and the best installable slot"
                                           " (default if --complete or --everything)")
            ("best",                  'b', "Consider the best installable slot only"
                                           " (default if --lazy)"),
            "best-or-installed"
            ),

    g_dependency_options(this, "Dependency Options", "Control which dependencies are followed."),
    a_follow_installed_build_dependencies(&g_dependency_options, "follow-installed-build-dependencies", 'D',
            "Follow build dependencies for installed packages (default if --complete or --everything)", true),
    a_ignore_installed_dependencies(&g_dependency_options, "ignore-installed-dependencies", 'd',
            "Ignore dependencies (except compiled-against dependencies, which are already taken) "
            "for installed packages. (default if --lazy)", true),


    g_suggestion_options(this, "Suggestion Options", "Control whether suggestions are taken. Suggestions that are "
            "already installed are instead treated as hard dependencies."),
    a_suggestions(&g_suggestion_options, "suggestions", '\0', "How to treat suggestions and recommendations",
            args::EnumArg::EnumArgOptions
            ("ignore",                     "Ignore suggestions")
            ("display",                    "Display suggestions, but do not take them unless explicitly told to do so")
            ("take",                       "Take all suggestions"),
            "display"),
    a_recommendations(&g_suggestion_options, "recommendations", '\0', "How to treat recommendations",
            args::EnumArg::EnumArgOptions
            ("ignore",                     "Ignore recommendations")
            ("display",                    "Display recommendations, but do not take them unless explicitly told to do so")
            ("take",                       "Take all recommendations"),
            "take"),
    a_take(&g_suggestion_options, "take", '\0', "Take any suggestion matching the supplied package specification"
            " (e.g. --take 'app-vim/securemodelines' or --take 'app-vim/*')"),
    a_take_from(&g_suggestion_options, "take-from", '\0', "Take all suggestions made by any package matching the "
            "supplied package specification"),
    a_discard(&g_suggestion_options, "discard", '\0', "Discard any suggestion matching the supplied package specification"),
    a_discard_from(&g_suggestion_options, "discard-from", '\0', "Discard all suggestions made by any package matching the "
            "supplied package specification"),

//            g_package_options(this, "Package Selection Options", "Control which packages are selected."),
//            a_prefer(&g_package_options, "prefer", '\0', "If there is a choice, prefer the specified package names"),
//            a_avoid(&g_package_options, "avoid", '\0', "If there is a choice, avoid the specified package names"),
//
//            g_ordering_options(this, "Package Ordering Options", "Control the order in which packages are installed"),
//            a_early(&g_ordering_options, "early", '\0', "Try to install the specified package name as early as possible"),
//            a_late(&g_ordering_options, "late", '\0', "Try to install the specified package name as late as possible"),
//
//            g_preset_options(this, "Preset Options", "Preset various constraints."),
//            a_soft_preset(&g_preset_options, "soft-preset", '\0', "Preset a given constraint, but allow the resolver to "
//                    "override it. For example, --soft-preset cat/pkg::installed will tell the resolver to use the installed "
//                    "cat/pkg if possible."),
//            a_fixed_preset(&g_preset_options, "fixed-preset", '\0', "Preset a given constraint, and do not allow the resolver to "
//                    "override it. For example, --fixed-preset cat/pkg::installed will force the resolver to use the installed "
//                    "cat/pkg, generating an error if it cannot."),
//
    g_destination_options(this, "Destination Options", "Control to which destinations packages are installed. "
            "If no options from this group are selected, install only to /. Otherwise, install to all of the "
            "specified destinations, and install to / as necessary to satisfy build dependencies."),
//            a_fetch(&g_destination_options, "fetch", 'f', "Only fetch packages, do not install anything", true),
    a_create_binaries(&g_destination_options, "create-binaries", 'b', "Create binary packages in the specified "
            "repository. Specify multiple times if different binary destination types are required."),
    a_no_binaries_for(&g_destination_options, "no-binaries-for", 'B', "Do not create binary packages from any "
            "origin ID matching this spec. May be specified multiple times. (e.g. --no-binaries-for '*/*::accounts' "
            "--no-binaries-for '*/*::virtuals')"),
//            a_install_to_chroot(&g_destination_options, "install-to-chroot", '\0', "Install packages to the environment-configured chroot", true),
    a_install_to_root(&g_destination_options, "install-to-root", '/', "Install packages to /", true),

    g_dump_options(this, "Dump Options", "Dump the resolver's state to stdout after completion, or when an "
            "error occurs. For debugging purposes; produces rather a lot of noise."),
    a_dump(&g_dump_options, "dump", '\0', "Dump debug output", true),
    a_dump_restarts(&g_dump_options, "dump-restarts", '\0', "Dump restarts", true)
{
}

ResolveCommandLineDisplayOptions::ResolveCommandLineDisplayOptions(args::ArgsHandler * const h) :
    ArgsSection(h, "Display Options"),
    g_display_options(this, "Display Options", "Options relating to the resolution display."),
//    a_show_option_descriptions(&g_display_options, "show-option-descriptions", '\0',
//            "Whether to display descriptions for package options",
//            args::EnumArg::EnumArgOptions
//            ("none",                       "Don't show any descriptions")
//            ("new",                        "Show for any new options")
//            ("changed",                    "Show for new or changed options")
//            ("all",                        "Show all options"),
//            "changed"
//            ),
    a_show_descriptions(&g_display_options, "show-descriptions", '\0',
            "Whether to display package descriptions",
            args::EnumArg::EnumArgOptions
            ("none",                       "Don't show any descriptions")
            ("new",                        "Show for new packages")
            ("all",                        "Show for all packages"),
            "new"
            ),
    g_explanations(this, "Explanations", "Options requesting the resolver explain a particular decision "
            "that it made"),
    a_explain(&g_explanations, "explain", '\0', "Explain why the resolver made a particular decision. The "
            "argument is a package dependency specification, so --explain dev-libs/boost or --explain qt:3"
            " or even --explain '*/*' (although --dump is a better way of getting highly noisy debug output)."),
    a_show_all_jobs(&g_explanations, "show-all-jobs", '\0', "Show all jobs that will be executed, rather than "
            "just installs.", true)
{
}

ResolveCommandLineExecutionOptions::ResolveCommandLineExecutionOptions(args::ArgsHandler * const h) :
    ArgsSection(h, "Execution Options"),

    g_world_options(this, "World Options", "Options controlling how the 'world' set is modified"),
    a_preserve_world(&g_world_options, "preserve-world", '1', "Do not modify the 'world' set", true),

    g_failure_options(this, "Failure Options", "Failure handling options."),
    a_continue_on_failure(&g_failure_options, "continue-on-failure", '\0',
            "Whether to continue after an error occurs",
            args::EnumArg::EnumArgOptions
            ("never",                      "Never")
            ("if-satisfied",               "If remaining packages' dependencies are satisfied")
            ("if-independent",             "If remaining packages do not depend upon any failing package")
            ("always",                     "Always (dangerous)"),
            "never"),

    g_phase_options(this, "Phase Options", "Options controlling which phases to execute. No sanity checking "
            "is done, allowing you to shoot as many feet off as you desire. Phase names do not have the "
            "src_, pkg_ or builtin_ prefix, so 'init', 'preinst', 'unpack', 'merge', 'strip' etc."),
    a_skip_phase(&g_phase_options, "skip-phase", '\0', "Skip the named phases"),
    a_abort_at_phase(&g_phase_options, "abort-at-phase", '\0', "Abort when a named phase is encountered"),
    a_skip_until_phase(&g_phase_options, "skip-until-phase", '\0', "Skip every phase until a named phase is encountered"),
    a_change_phases_for(&g_phase_options, "change-phases-for", '\0',
            "Control to which package or packages these phase options apply",
            args::EnumArg::EnumArgOptions
            ("all",                        "All packages")
            ("first",                      "Only the first package on the list")
            ("last",                       "Only the last package on the list"),
            "all")
{
}

ResolveCommandLineProgramOptions::ResolveCommandLineProgramOptions(args::ArgsHandler * const h) :
    ArgsSection(h, "Program Options"),
    g_program_options(this, "Program Options", "Options controlling which programs are used to carry out "
            "various tasks. Any replacement to the standard program must provide exactly the same interface. "
            "In all cases, $CAVE can be used to get the path of the main 'cave' executable."),
    a_display_resolution_program(&g_program_options, "display-resolution-program", '\0', "The program used to display "
            "the resolution. Defaults to '$CAVE display-resolution'."),
    a_execute_resolution_program(&g_program_options, "execute-resolution-program", '\0', "The program used to execute "
            "the resolution. Defaults to '$CAVE execute-resolution'."),
    a_perform_program(&g_program_options, "perform-program", '\0', "The program used to perform "
            "actions. Defaults to '$CAVE perform'."),
    a_update_world_program(&g_program_options, "update-world-program", '\0', "The program used to perform "
            "world updates. Defaults to '$CAVE update-world'.")
{
}

ResolveCommandLineImportOptions::ResolveCommandLineImportOptions(args::ArgsHandler * const h) :
    ArgsSection(h, "Import Options"),
    g_import_options(this, "Import Options", "Options controlling additional imported packages. These options "
            "should not be specified manually; they are for use by 'cave import'."),
    a_unpackaged_repository_params(&g_import_options, "unpackaged-repository-params", '\0', "Specifies "
            "the parameters used to construct an unpackaged repository, for use by 'cave import'.")
{
}

void
ResolveCommandLineImportOptions::apply(const std::tr1::shared_ptr<Environment> & env) const
{
    if (! a_unpackaged_repository_params.specified())
        return;

    std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
    for (args::StringSetArg::ConstIterator a(a_unpackaged_repository_params.begin_args()),
            a_end(a_unpackaged_repository_params.end_args()) ;
            a != a_end ; ++a)
    {
        std::string::size_type p(a->find('='));
        if (std::string::npos == p)
            throw InternalError(PALUDIS_HERE, "no = in '" + stringify(*a));
        keys->insert(a->substr(0, p), a->substr(p + 1));
    }

    std::tr1::shared_ptr<Repository> repo(RepositoryFactory::get_instance()->create(env.get(),
                std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
    env->package_database()->add_repository(10, repo);
}

ResolveCommandLine::ResolveCommandLine() :
    resolution_options(this),
    execution_options(this),
    display_options(this),
    program_options(this)
{
    add_usage_line("[ -x|--execute ] [ -z|--lazy or -c|--complete or -e|--everything ] spec ...");
    add_usage_line("[ -x|--execute ] [ -z|--lazy or -c|--complete or -e|--everything ] set");
}

std::string
ResolveCommandLine::app_name() const
{
    return "cave resolve";
}

std::string
ResolveCommandLine::app_synopsis() const
{
    return "Display how to resolve one or more targets, and possibly then "
        "perform that resolution.";
}

std::string
ResolveCommandLine::app_description() const
{
    return "Displays how to resolve one or more targets. If instructed, then "
        "executes the relevant install and uninstall actions to perform that "
        "resolution.";
}

void
ResolveCommandLineResolutionOptions::apply_shortcuts()
{
    if (a_lazy.specified() +
            a_complete.specified() +
            a_everything.specified() > 1)
        throw args::DoHelp("At most one of '--" + a_lazy.long_name() + "', '--" + a_complete.long_name()
                + "' or '--" + a_everything.long_name() + "' may be specified");

    if (a_lazy.specified())
    {
        if (! a_target_slots.specified())
            a_target_slots.set_argument("best");
        if (! a_slots.specified())
            a_slots.set_argument("best");
        if (! a_ignore_installed_dependencies.specified())
            a_ignore_installed_dependencies.set_specified(true);
    }

    if (a_complete.specified())
    {
        if (! a_keep.specified())
            a_keep.set_argument("if-same");
        if (! a_target_slots.specified())
            a_target_slots.set_argument("all");
        if (! a_slots.specified())
            a_slots.set_argument("all");
        if (! a_follow_installed_build_dependencies.specified())
            a_follow_installed_build_dependencies.set_specified(true);
        if (! a_reinstall_scm.specified())
            a_reinstall_scm.set_argument("weekly");
    }

    if (a_everything.specified())
    {
        if (! a_keep.specified())
            a_keep.set_argument("if-transient");
        if (! a_keep_targets.specified())
            a_keep_targets.set_argument("if-transient");
        if (! a_target_slots.specified())
            a_target_slots.set_argument("all");
        if (! a_slots.specified())
            a_slots.set_argument("all");
        if (! a_follow_installed_build_dependencies.specified())
            a_follow_installed_build_dependencies.set_specified(true);
    }
}

void
ResolveCommandLineResolutionOptions::verify(const std::tr1::shared_ptr<const Environment> & env)
{
    if (a_create_binaries.specified())
    {
        for (args::StringSetArg::ConstIterator a(a_create_binaries.begin_args()),
                a_end(a_create_binaries.end_args()) ;
                a != a_end ; ++a)
        {
            std::tr1::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(RepositoryName(*a)));
            if (repo->installed_root_key() || ! repo->destination_interface())
                throw args::DoHelp("Repository '" + *a + "' not suitable for --" + a_create_binaries.long_name());
        }
    }
}

