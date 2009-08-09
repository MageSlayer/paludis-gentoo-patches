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
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/destinations.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <map>

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

        args::ArgsGroup g_convenience_options;
        args::SwitchArg a_lazy;
        args::SwitchArg a_complete;
        args::SwitchArg a_everything;

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

        args::ArgsGroup g_explanations;
        args::StringSetArg a_explain;

//        args::ArgsGroup g_phase_options;
//        args::StringSetArg a_skip_phase;
//        args::StringSetArg a_abort_at_phase;
//        args::StringSetArg a_skip_until_phase;
//        args::EnumArg a_change_phases_for;

        args::ArgsGroup g_keep_options;
        args::EnumArg a_keep_targets;
        args::EnumArg a_keep;
        args::EnumArg a_reinstall_scm;
//        args::SwitchArg a_reinstall_for_removals;

        args::ArgsGroup g_slot_options;
        args::EnumArg a_target_slots;
        args::EnumArg a_slots;

        args::ArgsGroup g_dependency_options;
        args::SwitchArg a_follow_installed_build_dependencies;
        args::SwitchArg a_ignore_installed_dependencies;

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

            g_explanations(this, "Explanations", "Options requesting the resolver explain a particular decision "
                    "that it made"),
            a_explain(&g_explanations, "explain", '\0', "Explain why the resolver made a particular decision. The "
                    "argument is a package dependency specification, so --explain dev-libs/boost or --explain qt:3"
                    " or even --explain '*/*' (although --dump is a better way of getting highly noisy debug output)."),

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
                    ("weekly",                'w', "If they were installed more than a week ago")
                    ("never",                 'n', "Never (default if --lazy)"),
                    "weekly"
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
            const std::tr1::shared_ptr<const PackageID> id((*c)->decision()->if_package_id());
            if (id)
                std::cout << "* " << id->canonical_form(idcf_no_version) << " " << id->canonical_form(idcf_version)
                    << " to " << *(*c)->destinations() << std::endl;
            else
                throw InternalError(PALUDIS_HERE, "why did that happen?");
        }

        std::cout << std::endl;
    }

    struct ReasonDisplayer
    {
        void visit(const TargetReason &)
        {
            std::cout << "it is a target";
        }

        void visit(const DependencyReason & reason)
        {
            std::cout << "of dependency " << reason.sanitised_dependency() << " from " << *reason.from_id();
        }

        void visit(const PresetReason &)
        {
            std::cout << "of a preset";
        }

        void visit(const SetReason & reason)
        {
            std::cout << "of set " << reason.set_name() << ", which is because ";
            reason.reason_for_set()->accept(*this);
        }
    };

    void display_explanations(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<Resolver> & resolver,
            const ResolveCommandLine & cmdline)
    {
        Context context("When displaying explanations:");

        if (cmdline.a_explain.begin_args() == cmdline.a_explain.end_args())
            return;

        std::cout << "Explaining requested decisions:" << std::endl << std::endl;

        for (args::StringSetArg::ConstIterator i(cmdline.a_explain.begin_args()), i_end(cmdline.a_explain.end_args()) ;
                i != i_end ; ++i)
        {
            bool any(false);
            PackageDepSpec spec(parse_user_package_dep_spec(*i, env.get(), UserPackageDepSpecOptions() + updso_allow_wildcards));
            for (Resolver::ResolutionsByQPN_SConstIterator r(resolver->begin_resolutions_by_qpn_s()),
                    r_end(resolver->end_resolutions_by_qpn_s()) ;
                    r != r_end ; ++r)
            {
                if (! r->second->decision()->if_package_id())
                {
                    /* really we want this to work for simple cat/pkg and
                     * cat/pkg:slot specs anyway, even if we chose nothing */
                    continue;
                }

                if (! match_package(*env, spec, *r->second->decision()->if_package_id(), MatchPackageOptions()))
                    continue;

                any = true;

                std::cout << "For " << r->first << ":" << std::endl;
                std::cout << "    The following constraints were in action:" << std::endl;
                for (Constraints::ConstIterator c(r->second->constraints()->begin()),
                        c_end(r->second->constraints()->end()) ;
                        c != c_end ; ++c)
                {
                    if ((*c)->is_blocker())
                        std::cout << "      * blocker " << (*c)->base_spec();
                    else
                        std::cout << "      * " << (*c)->base_spec();

                    switch ((*c)->use_installed())
                    {
                        case ui_if_same:
                            std::cout << ", use installed if same";
                            break;
                        case ui_never:
                            std::cout << ", never using installed";
                            break;
                        case ui_only_if_transient:
                            std::cout << ", using installed only if transient";
                            break;
                        case ui_if_same_version:
                            std::cout << ", use installed if same version";
                            break;
                        case ui_if_possible:
                            std::cout << ", use installed if possible";
                            break;

                        case last_ui:
                            break;
                    }

                    if ((*c)->to_destination_slash())
                        std::cout << ", installing to /";

                    std::cout << std::endl;
                    std::cout << "        because ";
                    ReasonDisplayer v;
                    (*c)->reason()->accept(v);
                    std::cout << std::endl;
                }
                std::cout << "    The decision made was:" << std::endl;
                std::cout << "        Use " << *r->second->decision()->if_package_id() << std::endl;
                if (r->second->destinations()->slash())
                {
                    std::cout << "        Install to / using repository " << r->second->destinations()->slash()->repository() << std::endl;
                    if (! r->second->destinations()->slash()->replacing()->empty())
                        for (PackageIDSequence::ConstIterator x(r->second->destinations()->slash()->replacing()->begin()),
                                x_end(r->second->destinations()->slash()->replacing()->end()) ;
                                x != x_end ; ++x)
                            std::cout << "            Replacing " << **x << std::endl;
                }
                std::cout << std::endl;
            }

            if (! any)
                throw args::DoHelp("There is nothing matching '" + *i + "' in the resolution set.");
        }
    }

    void dump_if_requested(
            const std::tr1::shared_ptr<Environment> &,
            const std::tr1::shared_ptr<Resolver> & resolver,
            const ResolveCommandLine & cmdline)
    {
        Context context("When dumping the resolver:");

        if (! cmdline.a_dump.specified())
            return;

        std::cout << "Dumping resolutions by QPN:S:" << std::endl << std::endl;

        for (Resolver::ResolutionsByQPN_SConstIterator c(resolver->begin_resolutions_by_qpn_s()),
                c_end(resolver->end_resolutions_by_qpn_s()) ;
                c != c_end ; ++c)
        {
            std::cout << c->first << std::endl;
            std::cout << "  = " << *c->second << std::endl;
            if (cmdline.a_dump_dependencies.specified())
                for (SanitisedDependencies::ConstIterator d(c->second->sanitised_dependencies()->begin()),
                        d_end(c->second->sanitised_dependencies()->end()) ;
                        d != d_end ; ++d)
                    std::cout << "  -> " << *d << std::endl;
        }

        std::cout << std::endl;
    }

    UseInstalled use_installed_from_cmdline(const args::EnumArg & a, const bool is_set)
    {
        if (a.argument() == "auto")
            return is_set ? ui_if_same : ui_never;
        else if (a.argument() == "never")
            return ui_never;
        else if (a.argument() == "if-transient")
            return ui_only_if_transient;
        else if (a.argument() == "if-same")
            return ui_if_same;
        else if (a.argument() == "if-same-version")
            return ui_if_same_version;
        else if (a.argument() == "if-possible")
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
            return use_installed_from_cmdline(cmdline.a_keep, false);
        }

        UseInstalled visit(const TargetReason &) const
        {
            return use_installed_from_cmdline(cmdline.a_keep_targets, from_set);
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

    int reinstall_scm_days(const ResolveCommandLine & cmdline)
    {
        if (cmdline.a_reinstall_scm.argument() == "always")
            return 0;
        else if (cmdline.a_reinstall_scm.argument() == "daily")
            return 1;
        else if (cmdline.a_reinstall_scm.argument() == "weekly")
            return 7;
        else if (cmdline.a_reinstall_scm.argument() == "never")
            return -1;
        else
            throw args::DoHelp("Don't understand argument '" + cmdline.a_reinstall_scm.argument() + "' to '--"
                    + cmdline.a_reinstall_scm.long_name() + "'");
    }

    bool is_scm_name(const QualifiedPackageName & n)
    {
        std::string pkg(stringify(n.package()));
        switch (pkg.length())
        {
            case 0:
            case 1:
            case 2:
            case 3:
                return false;

            default:
                if (0 == pkg.compare(pkg.length() - 6, 6, "-darcs"))
                    return true;

            case 5:
                if (0 == pkg.compare(pkg.length() - 5, 5, "-live"))
                    return true;

            case 4:
                if (0 == pkg.compare(pkg.length() - 4, 4, "-cvs"))
                    return true;
                if (0 == pkg.compare(pkg.length() - 4, 4, "-svn"))
                    return true;
                return false;
        }
    }

    bool is_scm_older_than(const std::tr1::shared_ptr<const PackageID> & id, const int n)
    {
        if (id->version().is_scm() || is_scm_name(id->name()))
        {
            static time_t current_time(time(0)); /* static to avoid weirdness */
            time_t installed_time(current_time);
            if (id->installed_time_key())
                installed_time = id->installed_time_key()->value();

            return (current_time - installed_time) > (24 * 60 * 60 * n);
        }
        else
            return false;
    }

    bool installed_is_scm_older_than(const Environment * const env, const QPN_S & q, const int n)
    {
        Context context("When working out whether '" + stringify(q) + "' has installed SCM packages:");

        const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                    generator::Package(q.package()) |
                    q.make_slot_filter() |
                    filter::SupportsAction<InstalledAction>())]);

        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if (is_scm_older_than(*i, n))
                return true;
        }

        return false;
    }

    typedef std::map<QPN_S, std::tr1::shared_ptr<Constraints> > InitialConstraints;

    const std::tr1::shared_ptr<Constraints> make_initial_constraints_for(
            const Environment * const env,
            const ResolveCommandLine & cmdline,
            const QPN_S & qpn_s)
    {
        const std::tr1::shared_ptr<Constraints> result(new Constraints);

        int n(reinstall_scm_days(cmdline));
        if ((-1 != n) && installed_is_scm_older_than(env, qpn_s, n))
        {
            result->add(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                                value_for<n::base_spec>(make_package_dep_spec(PartiallyMadePackageDepSpecOptions()).package(qpn_s.package())),
                                value_for<n::is_blocker>(false),
                                value_for<n::nothing_is_fine_too>(false),
                                value_for<n::reason>(make_shared_ptr(new PresetReason)),
                                value_for<n::to_destination_slash>(false),
                                value_for<n::use_installed>(ui_only_if_transient)
                                ))));
        }

        return result;
    }

    const std::tr1::shared_ptr<Constraints> initial_constraints_for_fn(
            const Environment * const env,
            const ResolveCommandLine & cmdline,
            const InitialConstraints & initial_constraints,
            const QPN_S & qpn_s)
    {
        InitialConstraints::const_iterator i(initial_constraints.find(qpn_s));
        if (i == initial_constraints.end())
            return make_initial_constraints_for(env, cmdline, qpn_s);
        else
            return i->second;
    }

    struct IsTargetVisitor
    {
        bool visit(const DependencyReason &) const
        {
            return false;
        }

        bool visit(const PresetReason &) const
        {
            return false;
        }

        bool visit(const TargetReason &) const
        {
            return true;
        }

        bool visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<bool>(*this);
        }
    };

    bool is_target(const std::tr1::shared_ptr<const Reason> & reason)
    {
        IsTargetVisitor v;
        return reason->accept_returning<bool>(v);
    }

    struct SlotNameFinder
    {
        std::tr1::shared_ptr<SlotName> visit(const SlotExactRequirement & s)
        {
            return make_shared_ptr(new SlotName(s.slot()));
        }

        std::tr1::shared_ptr<SlotName> visit(const SlotAnyUnlockedRequirement &)
        {
            return make_null_shared_ptr();
        }

        std::tr1::shared_ptr<SlotName> visit(const SlotAnyLockedRequirement &)
        {
            return make_null_shared_ptr();
        }
    };

    const std::tr1::shared_ptr<QPN_S_Sequence>
    get_qpn_s_s_for_fn(const Environment * const env,
            const ResolveCommandLine & cmdline,
            const PackageDepSpec & spec,
            const std::tr1::shared_ptr<const Reason> & reason)
    {
        std::tr1::shared_ptr<QPN_S_Sequence> result(new QPN_S_Sequence);

        std::tr1::shared_ptr<SlotName> exact_slot;

        if (spec.slot_requirement_ptr())
        {
            SlotNameFinder f;
            exact_slot = spec.slot_requirement_ptr()->accept_returning<std::tr1::shared_ptr<SlotName> >(f);
        }

        if (exact_slot)
            result->push_back(QPN_S(spec, exact_slot));
        else
        {
            std::tr1::shared_ptr<QPN_S> best;
            std::list<QPN_S> installed;

            const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                        generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements) |
                        filter::SupportsAction<InstallAction>() |
                        filter::NotMasked())]);

            if (! ids->empty())
                best = make_shared_ptr(new QPN_S(*ids->begin()));

            const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*env)[selection::BestVersionInEachSlot(
                        generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements) |
                        filter::SupportsAction<InstalledAction>())]);

            for (PackageIDSequence::ConstIterator i(installed_ids->begin()), i_end(installed_ids->end()) ;
                    i != i_end ; ++i)
                installed.push_back(QPN_S(*i));

            const args::EnumArg & arg(is_target(reason) ? cmdline.a_target_slots : cmdline.a_slots);

            if (! best)
                std::copy(installed.begin(), installed.end(), result->back_inserter());
            else if (arg.argument() == "best-or-installed")
            {
                if (installed.end() == std::find(installed.begin(), installed.end(), *best))
                    result->push_back(*best);
                else
                    std::copy(installed.begin(), installed.end(), result->back_inserter());
            }
            else if (arg.argument() == "installed-or-best")
            {
                if (installed.empty())
                    result->push_back(*best);
                else
                    std::copy(installed.begin(), installed.end(), result->back_inserter());
            }
            else if (arg.argument() == "all")
            {
                if (installed.end() == std::find(installed.begin(), installed.end(), *best))
                    result->push_back(*best);
                std::copy(installed.begin(), installed.end(), result->back_inserter());
            }
            else if (arg.argument() == "best")
                result->push_back(*best);
            else
                throw args::DoHelp("Don't understand argument '" + arg.argument() + "' to '--"
                        + arg.long_name() + "'");
        }

        return result;
    }

    struct IsSuggestionVisitor
    {
        bool is_suggestion;

        IsSuggestionVisitor() :
            is_suggestion(true)
        {
        }

        void visit(const DependencyRequiredLabel &)
        {
            is_suggestion = false;
        }

        void visit(const DependencyRecommendedLabel &)
        {
            is_suggestion = false;
        }

        void visit(const DependencySuggestedLabel &)
        {
        }
    };

    struct IsTypeDepVisitor
    {
        bool is_build_dep;
        bool is_compiled_against_dep;

        IsTypeDepVisitor() :
            is_build_dep(true),
            is_compiled_against_dep(false)
        {
        }

        void visit(const DependencyBuildLabel &)
        {
        }

        void visit(const DependencyRunLabel &)
        {
            is_build_dep = false;
        }

        void visit(const DependencyPostLabel &)
        {
            is_build_dep = false;
        }

        void visit(const DependencyInstallLabel &)
        {
        }

        void visit(const DependencyCompileLabel &)
        {
            is_compiled_against_dep = true;
        }
    };

    bool is_suggestion(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->suggest_labels()->empty())
            return false;

        IsSuggestionVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->suggest_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->suggest_labels()->end()),
                accept_visitor(v));
        return v.is_suggestion;
    }

    bool is_just_build_dep(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->type_labels()->empty())
            throw InternalError(PALUDIS_HERE, "not implemented");

        IsTypeDepVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->type_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->type_labels()->end()),
                accept_visitor(v));
        return v.is_build_dep;
    }

    bool is_compiled_against_dep(const SanitisedDependency & dep)
    {
        if (dep.active_dependency_labels()->type_labels()->empty())
            throw InternalError(PALUDIS_HERE, "not implemented");

        IsTypeDepVisitor v;
        std::for_each(indirect_iterator(dep.active_dependency_labels()->type_labels()->begin()),
                indirect_iterator(dep.active_dependency_labels()->type_labels()->end()),
                accept_visitor(v));
        return v.is_compiled_against_dep;
    }

    bool care_about_dep_fn(const Environment * const, const ResolveCommandLine & cmdline,
            const QPN_S &, const std::tr1::shared_ptr<const Resolution> & resolution,
            const SanitisedDependency & dep)
    {
        if (is_suggestion(dep))
            return false;

        if (resolution->decision()->is_installed())
        {
            if (! cmdline.a_follow_installed_build_dependencies.specified())
                if (is_just_build_dep(dep))
                    return false;
            if (cmdline.a_ignore_installed_dependencies.specified())
                if (! is_compiled_against_dep(dep))
                    return false;
        }

        return true;
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

    if (cmdline.a_lazy.specified() + cmdline.a_complete.specified() + cmdline.a_everything.specified() > 1)
        throw args::DoHelp("At most one of '--" + cmdline.a_lazy.long_name() + "', '--" + cmdline.a_complete.long_name()
                + "' or '--" + cmdline.a_everything.long_name() + "' may be specified");

    if (cmdline.a_lazy.specified())
    {
        if (! cmdline.a_target_slots.specified())
            cmdline.a_target_slots.set_argument("best");
        if (! cmdline.a_slots.specified())
            cmdline.a_slots.set_argument("best");
        if (! cmdline.a_reinstall_scm.specified())
            cmdline.a_reinstall_scm.set_argument("never");
        if (! cmdline.a_ignore_installed_dependencies.specified())
            cmdline.a_ignore_installed_dependencies.set_specified(true);
    }

    if (cmdline.a_complete.specified())
    {
        if (! cmdline.a_keep.specified())
            cmdline.a_keep.set_argument("if-same");
        if (! cmdline.a_target_slots.specified())
            cmdline.a_target_slots.set_argument("all");
        if (! cmdline.a_slots.specified())
            cmdline.a_slots.set_argument("all");
        if (! cmdline.a_follow_installed_build_dependencies.specified())
            cmdline.a_follow_installed_build_dependencies.set_specified(true);
    }

    if (cmdline.a_everything.specified())
    {
        if (! cmdline.a_keep.specified())
            cmdline.a_keep.set_argument("if-transient");
        if (! cmdline.a_keep_targets.specified())
            cmdline.a_keep_targets.set_argument("if-transient");
        if (! cmdline.a_target_slots.specified())
            cmdline.a_target_slots.set_argument("all");
        if (! cmdline.a_slots.specified())
            cmdline.a_slots.set_argument("all");
        if (! cmdline.a_follow_installed_build_dependencies.specified())
            cmdline.a_follow_installed_build_dependencies.set_specified(true);
    }

    int retcode(0);

    InitialConstraints initial_constraints;

    ResolverFunctions resolver_functions(make_named_values<ResolverFunctions>(
                value_for<n::care_about_dep_fn>(std::tr1::bind(&care_about_dep_fn,
                        env.get(), std::tr1::cref(cmdline), std::tr1::placeholders::_1,
                        std::tr1::placeholders::_2, std::tr1::placeholders::_3)),
                value_for<n::get_initial_constraints_for_fn>(std::tr1::bind(&initial_constraints_for_fn,
                        env.get(), std::tr1::cref(cmdline), std::tr1::cref(initial_constraints), std::tr1::placeholders::_1)),
                value_for<n::get_qpn_s_s_for_fn>(std::tr1::bind(&get_qpn_s_s_for_fn,
                        env.get(), std::tr1::cref(cmdline), std::tr1::placeholders::_1, std::tr1::placeholders::_2)),
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
                initial_constraints.insert(std::make_pair(e.qpn_s(), make_initial_constraints_for(
                                env.get(), cmdline, e.qpn_s()))).first->second->add(
                        e.suggested_preset());
                resolver = make_shared_ptr(new Resolver(env.get(), resolver_functions));
            }
        }

        display_resolution(env, resolver, cmdline);
        display_explanations(env, resolver, cmdline);
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

