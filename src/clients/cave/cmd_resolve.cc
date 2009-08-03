/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include "cmd_resolve.hh"
#include "exceptions.hh"

#include "command_command_line.hh"

#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/args/do_help.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/notifier_callback.hh>

#include <iostream>
#include <cstdlib>

using namespace paludis;
using namespace paludis::resolver;
using namespace cave;

using std::cout;
using std::endl;

namespace
{
    struct ResolveCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave resolve";
        }

        virtual std::string app_synopsis() const
        {
            return "Display how to resolve one or more targets, and possibly then "
                "perform that resolution.";
        }

        virtual std::string app_description() const
        {
            return "Displays how to resolve one or more targets. If instructed, then "
                "executes the relevant install and uninstall actions to perform that "
                "resolution.";
        }

//        args::ArgsGroup g_execution_options;
//        args::SwitchArg a_execute;
//        args::SwitchArg a_preserve_world;

//        args::ArgsGroup g_convenience_options;
//        args::SwitchArg a_lazy;
//        args::SwitchArg a_complete;
//        args::SwitchArg a_everything;

//        args::ArgsGroup g_resolution_options;
//        args::SwitchArg a_permit_slot_uninstalls;
//        args::SwitchArg a_permit_uninstalls;
//        args::SwitchArg a_permit_downgrades;
//        args::SwitchArg a_permit_unsafe_uninstalls;

//        args::ArgsGroup g_cleanup_options;
//        args::SwitchArg a_purge_unused_slots;
//        args::SwitchArg a_purge_unused_packages;

//        args::ArgsGroup g_failure_options;
//        args::EnumArg a_continue_on_failure;

//        args::ArgsGroup g_display_options;
//        args::EnumArg a_show_option_descriptions;
//        args::EnumArg a_show_descriptions;

//        args::ArgsGroup g_explanations;
//        args::StringSetArg a_explain;

//        args::ArgsGroup g_phase_options;
//        args::StringSetArg a_skip_phase;
//        args::StringSetArg a_abort_at_phase;
//        args::StringSetArg a_skip_until_phase;
//        args::EnumArg a_change_phases_for;

        args::ArgsGroup g_reinstall_options;
        args::EnumArg a_reinstall_targets;
        args::EnumArg a_reinstall;
        args::EnumArg a_reinstall_scm;
//        args::SwitchArg a_reinstall_for_removals;

//        args::ArgsGroup g_slot_options;
//        args::EnumArg a_target_slots;
//        args::EnumArg a_slots;

//        args::ArgsGroup g_dependency_options;
//        args::SwitchArg a_follow_installed_build_dependencies;
//        args::SwitchArg a_ignore_installed_dependencies;

//        args::ArgsGroup g_suggestion_options;
//        args::EnumArg a_suggestions;
//        args::EnumArg a_recommendations;
//        args::StringSetArg a_take;
//        args::StringSetArg a_take_from;
//        args::StringSetArg a_discard;
//        args::StringSetArg a_discard_from;

//        args::ArgsGroup g_package_options;
//        args::StringSetArg a_prefer;
//        args::StringSetArg a_avoid;

//        args::ArgsGroup g_ordering_options;
//        args::StringSetArg a_early;
//        args::StringSetArg a_late;

//        args::ArgsGroup g_preset_options;
//        args::StringSetArg a_soft_preset;
//        args::StringSetArg a_fixed_preset;

//        args::ArgsGroup g_destination_options;
//        args::SwitchArg a_fetch;
//        args::SwitchArg a_create_binaries;
//        args::SwitchArg a_install_to_chroot;
//        args::SwitchArg a_install_to_root;

//        args::ArgsGroup g_interactivity_options;
//        args::SwitchArg a_interactive;
//        args::SwitchArg a_interactive_slots;
//        args::SwitchArg a_interactive_decisions;
//        args::SwitchArg a_interactive_ordering;

        args::ArgsGroup g_dump_options;
        args::SwitchArg a_dump;
        args::SwitchArg a_dump_dependencies;

        ResolveCommandLine() :
//            g_execution_options(this, "Execution Options", "Control execution."),
//            a_execute(&g_execution_options, "execute", 'x', "Execute the suggested actions", true),
//            a_preserve_world(&g_execution_options, "preserve-world", '1', "Do not modify the 'world' set", true),
//
//            g_convenience_options(this, "Convenience Options", "Broad behaviour options."),
//            a_lazy(&g_convenience_options, "lazy", 'z', "Do as little work as possible.", true),
//            a_complete(&g_convenience_options, "complete", 'c', "Do all optional work.", true),
//            a_everything(&g_convenience_options, "everything", 'e', "Do all optional work, and also reinstall", true),
//
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
//            g_failure_options(this, "Failure Options", "Failure handling options."),
//            a_continue_on_failure(&g_failure_options, "continue-on-failure", '\0',
//                    "Whether to continue after an error occurs",
//                    args::EnumArg::EnumArgOptions
//                    ("if-fetching",                "Only if we are just fetching packages")
//                    ("never",                      "Never")
//                    ("if-satisfied",               "If remaining packages' dependencies are satisfied")
//                    ("if-independent",             "If remaining packages do not depend upon any failing package")
//                    ("always",                     "Always (dangerous)"),
//                    "if-fetching"),
//
//            g_display_options(this, "Display Options", "Options relating to the resolution display."),
//            a_show_option_descriptions(&g_display_options, "show-option-descriptions", '\0',
//                    "Whether to display descriptions for package options",
//                    args::EnumArg::EnumArgOptions
//                    ("none",                       "Don't show any descriptions")
//                    ("new",                        "Show for any new options")
//                    ("changed",                    "Show for new or changed options")
//                    ("all",                        "Show all options"),
//                    "changed"
//                    ),
//            a_show_descriptions(&g_display_options, "show-descriptions", '\0',
//                    "Whether to display package descriptions",
//                    args::EnumArg::EnumArgOptions
//                    ("none",                       "Don't show any descriptions")
//                    ("new",                        "Show for new packages")
//                    ("all",                        "Show for all packages"),
//                    "new"
//                    ),
//
//            g_explanations(this, "Explanations", "Options requesting the resolver explain a particular decision "
//                    "that it made"),
//            a_explain(&g_explanations, "explain", '\0', "Explain why the resolver made a particular decision. The "
//                    "argument is a package dependency specification, so --explain dev-libs/boost or --explain qt:3"
//                    " or even --explain '*/*' (although --dump is a better way of getting highly noisy debug output)."),
//
//            g_phase_options(this, "Phase Options", "Options controlling which phases to execute. No sanity checking "
//                    "is done, allowing you to shoot as many feet off as you desire. Phase names do not have the "
//                    "src_, pkg_ or builtin_ prefix, so 'init', 'preinst', 'unpack', 'merge', 'strip' etc."),
//            a_skip_phase(&g_phase_options, "skip-phase", '\0', "Skip the named phases"),
//            a_abort_at_phase(&g_phase_options, "abort-at-phase", '\0', "Abort when a named phase is encounted"),
//            a_skip_until_phase(&g_phase_options, "skip-until-phase", '\0', "Skip every phase until a named phase is encounted"),
//            a_change_phases_for(&g_phase_options, "change-phases-for", '\0',
//                    "Control to which package or packages these phase options apply",
//                    args::EnumArg::EnumArgOptions
//                    ("all",                        "All packages")
//                    ("first",                      "Only the first package on the list")
//                    ("last",                       "Only the last package on the list"),
//                    "all"),
//
            g_reinstall_options(this, "Reinstall Options", "Control whether packages are reinstalled."),
            a_reinstall_targets(&g_reinstall_options, "reinstall-targets", 'R',
                    "Select whether to reinstall target packages",
                    args::EnumArg::EnumArgOptions
                    ("auto",                  'x', "If the target is a set, if-same, otherwise always")
                    ("always",                'a', "Always")
                    ("unless-transient",      't', "Unless the target is transient (e.g. from 'importare')"
                                                   " (default if --everything)")
                    ("unless-same",           's', "Unless the target is the same (version, option flags)")
                    ("unless-same-version",   'v', "Unless the target is the same version")
                    ("if-necessary",          'i', "If necessary"),
                    "auto"
                    ),
            a_reinstall(&g_reinstall_options, "reinstall", 'r',
                    "Select whether to reinstall packages that are not targets",
                    args::EnumArg::EnumArgOptions
                    ("always",                'a', "Always")
                    ("unless-transient",      't', "Unless the target is transient (e.g. from 'importare')"
                                                   " (default if --everything)")
                    ("unless-same",           's', "Unless the target is the same (version, option flags)"
                                                   " (default if --complete)")
                    ("unless-same-version",   'v', "Unless the target is the same version")
                    ("if-necessary",          'i', "If necessary"),
                    "if-necessary"
                    ),
            a_reinstall_scm(&g_reinstall_options, "reinstall-scm", 's',
                    "Select whether to reinstall SCM packages that would otherwise not be reinstalled",
                    args::EnumArg::EnumArgOptions
                    ("always",                'a', "Always")
                    ("daily",                 'd', "If they were installed more than a day ago")
                    ("weekly",                'w', "If they were installed more than a week ago")
                    ("never",                 'n', "Never"),
                    "weekly"
                    ),
//            a_reinstall_for_removals(&g_reinstall_options, "reinstall-for-removals", '\0',
//                    "Select whether to rebuild packages if rebuilding would avoid an unsafe removal", true),
//
//            g_slot_options(this, "Slot Options", "Control which slots are considered."),
//            a_target_slots(&g_slot_options, "target-slots", 'S',
//                    "Which slots to consider for targets",
//                    args::EnumArg::EnumArgOptions
//                    ("best-or-installed",     'x', "Consider the best slot, if it is not installed, "
//                                                   "or all installed slots otherwise")
//                    ("installed-or-best",     'i', "Consider all installed slots, or the best "
//                                                   "installable slot if nothing is installed")
//                    ("all",                   'a', "Consider all installed slots and the best installable slot"
//                                                   " (default if --complete or --everything)")
//                    ("best",                  'b', "Consider the best installable slot only"
//                                                   " (default if --lazy)"),
//                    "best-or-installed"
//                    ),
//            a_slots(&g_slot_options, "slots", 's',
//                    "Which slots to consider for packages that are not targets",
//                    args::EnumArg::EnumArgOptions
//                    ("best-or-installed",     'x', "Consider the best slot, if it is not installed, "
//                                                   "or all installed slots otherwise")
//                    ("installed-or-best",     'i', "Consider all installed slots, or the best "
//                                                   "installable slot if nothing is installed")
//                    ("all",                   'a', "Consider all installed slots and the best installable slot"
//                                                   " (default if --complete or --everything)")
//                    ("best",                  'b', "Consider the best installable slot only"
//                                                   " (default if --lazy)"),
//                    "best-or-installed"
//                    ),
//
//            g_dependency_options(this, "Dependency Options", "Control which dependencies are followed."),
//            a_follow_installed_build_dependencies(&g_dependency_options, "follow-installed-build-dependencies", 'D',
//                    "Follow build dependencies for installed packages (default if --complete or --everything", true),
//            a_ignore_installed_dependencies(&g_dependency_options, "ignore-installed-dependencies", 'd',
//                    "Ignore (non-build) dependencies for installed packages (default if --lazy)", true),
//
//            g_suggestion_options(this, "Suggestion Options", "Control whether suggestions are taken. Suggestions that are "
//                    "already installed are instead treated as hard dependencies."),
//            a_suggestions(&g_suggestion_options, "suggestions", '\0', "How to treat suggestions and recommendations",
//                    args::EnumArg::EnumArgOptions
//                    ("ignore",                     "Ignore suggestions")
//                    ("display",                    "Display suggestions, but do not take them unless explicitly told to do so")
//                    ("take",                       "Take all suggestions"),
//                    "display"),
//            a_recommendations(&g_suggestion_options, "recommendations", '\0', "How to treat recommendations",
//                    args::EnumArg::EnumArgOptions
//                    ("ignore",                     "Ignore recommendations")
//                    ("display",                    "Display recommendations, but do not take them unless explicitly told to do so")
//                    ("take",                       "Take all recommendations"),
//                    "take"),
//            a_take(&g_suggestion_options, "take", '\0', "Take any suggestion matching the supplied package specification"
//                    " (e.g. --take 'app-vim/securemodelines' or --take 'app-vim/*')"),
//            a_take_from(&g_suggestion_options, "take-from", '\0', "Take all suggestions made by any package matching the "
//                    "supplied package specification"),
//            a_discard(&g_suggestion_options, "discard", '\0', "Discard any suggestion matching the supplied package specification"),
//            a_discard_from(&g_suggestion_options, "discard-from", '\0', "Discard all suggestions made by any package matching the "
//                    "supplied package specification"),
//
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
//            g_destination_options(this, "Destination Options", "Control to which destinations packages are installed. "
//                    "If no options from this group are selected, install only to /. Otherwise, install to all of the "
//                    "specified destinations, and install to / as necessary to satisfy build dependencies."),
//            a_fetch(&g_destination_options, "fetch", 'f', "Only fetch packages, do not install anything", true),
//            a_create_binaries(&g_destination_options, "create-binaries", '\0', "Create binary packages", true),
//            a_install_to_chroot(&g_destination_options, "install-to-chroot", '\0', "Install packages to the environment-configured chroot", true),
//            a_install_to_root(&g_destination_options, "install-to-root", '\0', "Install packages to /", true),

            g_dump_options(this, "Dump Options", "Dump the resolver's state to stdout after completion, or when an "
                    "error occurs. For debugging purposes; produces rather a lot of noise."),
            a_dump(&g_dump_options, "dump", '\0', "Dump debug output", true),
            a_dump_dependencies(&g_dump_options, "dump-dependencies", '\0', "If dumping, also dump the "
                    "sanitised dependencies selected for every package" , true)
        {
            add_usage_line("[ -x|--execute ] [ -z|--lazy or -c|--complete or -e|--everything ] spec ...");
            add_usage_line("[ -x|--execute ] [ -z|--lazy or -c|--complete or -e|--everything ] set");
        }
    };

    struct DisplayCallback
    {
        mutable Mutex mutex;
        mutable int width, metadata, steps;

        DisplayCallback() :
            width(0),
            metadata(0),
            steps(0)
        {
            std::cout << "Resolving: " << std::flush;
        }

        ~DisplayCallback()
        {
            std::cout << std::endl << std::endl;
        }

        void operator() (const NotifierCallbackEvent & event) const
        {
            event.accept(*this);
        }

        void visit(const NotifierCallbackGeneratingMetadataEvent &) const
        {
            Lock lock(mutex);
            ++metadata;
            update();
        }

        void visit(const NotifierCallbackResolverStepEvent &) const
        {
            Lock lock(mutex);
            ++steps;
            update();
        }

        void update() const
        {
            std::string s;
            if (0 != steps)
                s.append("steps: " + stringify(steps));

            if (0 != metadata)
            {
                if (! s.empty())
                    s.append(", ");
                s.append("metadata: " + stringify(metadata));
            }

            std::cout << std::string(width, '\010') << s << std::flush;
            width = s.length();
        }
    };

    void add_resolver_targets(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<Resolver> & resolver,
            const ResolveCommandLine & cmdline)
    {
        Context context("When adding targets from commandline:");

        if (cmdline.begin_parameters() == cmdline.end_parameters())
            throw args::DoHelp("Must specify at least one target");

        bool seen_sets(false), seen_packages(false);
        for (ResolveCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
                p != p_end ; ++p)
        {
            try
            {
                resolver->add_target(parse_user_package_dep_spec(*p, env.get(),
                            UserPackageDepSpecOptions() + updso_throw_if_set));
                if (seen_sets)
                    throw args::DoHelp("Cannot specify both set and package targets");
                seen_packages = true;
            }
            catch (const GotASetNotAPackageDepSpec &)
            {
                if (seen_packages)
                    throw args::DoHelp("Cannot specify both set and package targets");
                if (seen_sets)
                    throw args::DoHelp("Cannot specify multiple set targets");

                resolver->add_target(SetName(*p));
                seen_sets = true;
            }
        }
    }

    void display_resolution(
            const std::tr1::shared_ptr<Environment> &,
            const std::tr1::shared_ptr<Resolver> & resolver,
            const ResolveCommandLine &)
    {
        Context context("When displaying chosen resolution:");

        for (Resolver::ConstIterator c(resolver->begin()), c_end(resolver->end()) ;
                c != c_end ; ++c)
        {
            const std::tr1::shared_ptr<const PackageID> id((*c)->decision()->package_id());
            std::cout << "* " << id->canonical_form(idcf_no_version) << " " << id->canonical_form(idcf_version)
                << " to " << *(*c)->destinations() << std::endl;
        }

        std::cout << std::endl;
    }

    void dump_if_requested(
            const std::tr1::shared_ptr<Environment> &,
            const std::tr1::shared_ptr<Resolver> & resolver,
            const ResolveCommandLine & cmdline)
    {
        Context context("When dumping the resolver:");

        if (cmdline.a_dump.specified())
            resolver->dump(std::cerr, cmdline.a_dump_dependencies.specified());
    }

    UseInstalled use_installed_from_cmdline(const args::EnumArg & a, const bool is_set)
    {
        if (a.argument() == "auto")
            return is_set ? ui_if_same : ui_never;
        else if (a.argument() == "always")
            return ui_never;
        else if (a.argument() == "unless-transient")
            return ui_only_if_transient;
        else if (a.argument() == "unless-same")
            return ui_if_same;
        else if (a.argument() == "unless-same-version")
            return ui_if_same_version;
        else if (a.argument() == "if-necessary")
            return ui_if_possible;
        else
            throw args::DoHelp("Don't understand argument '" + a.argument() + "' to '--" + a.long_name() + "'");
    }

    struct UseInstalledVisitor
    {
        const ResolveCommandLine & cmdline;
        const bool from_set;

        UseInstalledVisitor(const ResolveCommandLine & c, const bool f) :
            cmdline(c),
            from_set(f)
        {
        }

        UseInstalled visit(const DependencyReason &) const
        {
            return use_installed_from_cmdline(cmdline.a_reinstall, false);
        }

        UseInstalled visit(const TargetReason &) const
        {
            return use_installed_from_cmdline(cmdline.a_reinstall_targets, from_set);
        }

        UseInstalled visit(const PresetReason &) const
        {
            return ui_if_possible;
        }

        UseInstalled visit(const SetReason & r) const
        {
            UseInstalledVisitor v(cmdline, true);
            return r.reason_for_set()->accept_returning<UseInstalled>(v);
        }
    };

    UseInstalled use_installed_fn(const ResolveCommandLine & cmdline,
            const QPN_S &,
            const PackageDepSpec &,
            const std::tr1::shared_ptr<const Reason> & reason)
    {
        UseInstalledVisitor v(cmdline, false);
        return reason->accept_returning<UseInstalled>(v);
    }
}

bool
ResolveCommand::important() const
{
    return true;
}

int
ResolveCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    ResolveCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_RESOLVE_OPTIONS", "CAVE_RESOLVE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    int retcode(0);

    ResolverFunctions resolver_functions(make_named_values<ResolverFunctions>(
                value_for<n::get_use_installed_fn>(std::tr1::bind(&use_installed_fn,
                        std::tr1::cref(cmdline), std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3))
                ));
    std::tr1::shared_ptr<Resolver> resolver(new Resolver(env.get(), resolver_functions));
    try
    {
        while (true)
        {
            try
            {
                DisplayCallback display_callback;
                ScopedNotifierCallback display_callback_holder(env.get(),
                        NotifierCallbackFunction(std::tr1::cref(display_callback)));
                add_resolver_targets(env, resolver, cmdline);
                resolver->resolve();
                break;
            }
            catch (const SuggestRestart & e)
            {
                std::cout << "Restarting: for '" << e.qpn_s() << "' we had chosen '" << *e.previous_decision()
                    << "' but new constraint '" << *e.problematic_constraint() << "' means we now want '"
                    << *e.new_decision() << "' instead" << std::endl;
                std::tr1::shared_ptr<Resolver> new_resolver(new Resolver(env.get(), resolver_functions));
                new_resolver->copy_initial_constraints_from(*resolver);
                new_resolver->add_initial_constraint(e.qpn_s(), e.suggested_preset());
                resolver = new_resolver;
            }
        }

        display_resolution(env, resolver, cmdline);
    }
    catch (...)
    {
        dump_if_requested(env, resolver, cmdline);
        throw;
    }

    dump_if_requested(env, resolver, cmdline);

    return retcode;
}

std::tr1::shared_ptr<args::ArgsHandler>
ResolveCommand::make_doc_cmdline()
{
    return make_shared_ptr(new ResolveCommandLine);
}

