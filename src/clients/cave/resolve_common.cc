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

#include "resolve_common.hh"
#include "cmd_resolve_display_callback.hh"
#include "cmd_resolve_dump.hh"
#include "cmd_display_resolution.hh"
#include "cmd_execute_resolution.hh"
#include "exceptions.hh"
#include "command_command_line.hh"
#include "match_qpns.hh"
#include "not_strongly_masked.hh"

#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/system.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/string_list_stream.hh>
#include <paludis/util/thread.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/args/do_help.hh>
#include <paludis/args/escape.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/resolver/labels_classifier.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/selection_cache.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <paludis/elike_slot_requirement.hh>

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <list>
#include <map>

#include "config.h"

using namespace paludis;
using namespace paludis::resolver;
using namespace cave;

using std::cout;
using std::endl;

namespace
{
    typedef std::map<Resolvent, std::shared_ptr<Constraints> > InitialConstraints;
    typedef std::list<PackageDepSpec> PackageDepSpecList;

    bool can_make_binary_for(const std::shared_ptr<const PackageID> & id)
    {
        if (! id->behaviours_key())
            return true;
        return id->behaviours_key()->value()->end() == id->behaviours_key()->value()->find("unbinaryable");
    }

    struct DestinationTypesFinder
    {
        const Environment * const env;
        const ResolveCommandLineResolutionOptions & resolution_options;
        const std::shared_ptr<const PackageID> package_id;

        DestinationTypesFinder(
                const Environment * const e,
                const ResolveCommandLineResolutionOptions & c,
                const std::shared_ptr<const PackageID> & i) :
            env(e),
            resolution_options(c),
            package_id(i)
        {
        }

        DestinationTypes visit(const TargetReason &) const
        {
#ifdef ENABLE_PBINS
            if (resolution_options.a_make.argument() == "binaries")
            {
                if (package_id && can_make_binary_for(package_id))
                    return DestinationTypes() + dt_create_binary;
                else
                    return DestinationTypes();
            }
            else if (resolution_options.a_make.argument() == "install")
                return DestinationTypes() + dt_install_to_slash;
            else
                throw args::DoHelp("Don't understand argument '" + resolution_options.a_make.argument() + "' to '--"
                        + resolution_options.a_make.long_name() + "'");
#else
            return DestinationTypes() + dt_install_to_slash;
#endif
        }

        DestinationTypes visit(const DependentReason &) const
        {
            return DestinationTypes() + dt_install_to_slash;
        }

        DestinationTypes visit(const ViaBinaryReason &) const
        {
            return DestinationTypes();
        }

        DestinationTypes visit(const WasUsedByReason &) const
        {
            return DestinationTypes() + dt_install_to_slash;
        }

        DestinationTypes visit(const DependencyReason & dep) const
        {
            DestinationTypes extras;

#ifdef ENABLE_PBINS
            if (resolution_options.a_make.argument() == "binaries")
            {
                bool binary_if_possible(false);
                if (resolution_options.a_make_dependencies.argument() == "auto" ||
                        resolution_options.a_make_dependencies.argument() == "all")
                    binary_if_possible = true;
                else if (resolution_options.a_make_dependencies.argument() == "runtime")
                {
                    /* this will track run deps of build deps, which isn't
                     * really right... */
                    if (is_run_or_post_dep(dep.sanitised_dependency()))
                        binary_if_possible = true;
                }

                if (binary_if_possible && package_id && can_make_binary_for(package_id))
                    extras += dt_create_binary;
            }
#endif

            return (DestinationTypes() + dt_install_to_slash) | extras;
        }

        DestinationTypes visit(const PresetReason &) const
        {
            return DestinationTypes();
        }

        DestinationTypes visit(const LikeOtherDestinationTypeReason & r) const
        {
            return r.reason_for_other()->accept_returning<DestinationTypes>(*this);
        }

        DestinationTypes visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<DestinationTypes>(*this);
        }
    };

    DestinationTypes get_destination_types_for_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpec &,
            const std::shared_ptr<const PackageID> & id,
            const std::shared_ptr<const Reason> & reason)
    {
        DestinationTypesFinder f(env, resolution_options, id);
        return reason->accept_returning<DestinationTypes>(f);
    }

    FilteredGenerator make_destination_filtered_generator(
            const Environment * const,
            const ResolveCommandLineResolutionOptions &,
            const std::shared_ptr<const Generator> & all_binary_repos_generator,
            const Generator & g,
            const Resolvent & r)
    {
        switch (r.destination_type())
        {
            case dt_install_to_slash:
                return g | filter::InstalledAtRoot(FSEntry("/"));

            case dt_create_binary:
                if (all_binary_repos_generator)
                    return g & *all_binary_repos_generator;
                else
                    return generator::Nothing();

            case last_dt:
                break;
        }

        throw InternalError(PALUDIS_HERE, stringify(r.destination_type()));
    }

    FilteredGenerator make_destination_filtered_generator_with_resolution(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const std::shared_ptr<const Generator> & all_binary_repos_generator,
            const Generator & g,
            const std::shared_ptr<const Resolution> & r)
    {
        return make_destination_filtered_generator(env, resolution_options, all_binary_repos_generator, g, r->resolvent());
    }

    FilteredGenerator make_origin_filtered_generator(
            const Environment * const,
            const ResolveCommandLineResolutionOptions &,
            const std::shared_ptr<const Generator> & not_binary_repos_generator,
            const Generator & g,
            const std::shared_ptr<const Resolution> & r)
    {
        switch (r->resolvent().destination_type())
        {
            case dt_install_to_slash:
                return g;

            case dt_create_binary:
                return g & *not_binary_repos_generator;

            case last_dt:
                break;
        }

        throw InternalError(PALUDIS_HERE, "bad dt");
    }

    struct UnmaskableFilterHandler :
        AllFilterHandlerBase
    {
        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & id) const
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                    i != i_end ; ++i)
                if (not_strongly_masked(*i))
                    result->insert(*i);

            return result;
        }

        virtual std::string as_string() const
        {
            return "unmaskable";
        }
    };

    struct UnmaskableFilter :
        Filter
    {
        UnmaskableFilter() :
            Filter(std::make_shared<UnmaskableFilterHandler>())
        {
        }
    };

    Filter make_unmaskable_filter_fn(
            const Environment * const,
            const ResolveCommandLineResolutionOptions & cmdline,
            const std::shared_ptr<const Resolution> &)
    {
        if (cmdline.a_no_override_masks.specified())
            return filter::NotMasked();
        else
            return UnmaskableFilter();
    }

    const std::shared_ptr<const Sequence<std::string> > add_resolver_targets(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<Resolver> & resolver,
            const ResolveCommandLineResolutionOptions &,
            const std::shared_ptr<const Sequence<std::pair<std::string, std::string> > > & targets,
            bool & is_set)
    {
        Context context("When adding targets from commandline:");

        if (targets->empty())
            throw args::DoHelp("Must specify at least one target");

        const std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
        bool seen_sets(false), seen_packages(false);
        for (Sequence<std::pair<std::string, std::string> >::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
        {
            if (p->first.empty())
                continue;

            try
            {
                if ('!' == p->first.at(0))
                {
                    seen_packages = true;
                    PackageDepSpec s(parse_user_package_dep_spec(p->first.substr(1), env.get(), { }));
                    BlockDepSpec bs("!" + stringify(s), s, false);
                    result->push_back(stringify(bs));
                    resolver->add_target(bs, p->second);
                }
                else
                {
                    PackageDepSpec s(parse_user_package_dep_spec(p->first, env.get(),
                                { updso_throw_if_set }));
                    result->push_back(stringify(s));
                    resolver->add_target(s, p->second);
                    seen_packages = true;
                }
            }
            catch (const GotASetNotAPackageDepSpec &)
            {
                if (seen_sets)
                    throw args::DoHelp("Cannot specify multiple set targets");

                resolver->add_target(SetName(p->first), p->second);
                result->push_back(p->first);
                seen_sets = true;
            }
        }

        if (seen_sets + seen_packages > 1)
            throw args::DoHelp("Cannot specify set and non-set targets simultaneously");

        if (seen_sets)
            is_set = true;

        return result;
    }

    UseExisting use_existing_from_cmdline(const args::EnumArg & a, const bool is_set)
    {
        if (a.argument() == "auto")
            return is_set ? ue_if_same : ue_never;
        else if (a.argument() == "never")
            return ue_never;
        else if (a.argument() == "if-transient")
            return ue_only_if_transient;
        else if (a.argument() == "if-same")
            return ue_if_same;
        else if (a.argument() == "if-same-version")
            return ue_if_same_version;
        else if (a.argument() == "if-possible")
            return ue_if_possible;
        else
            throw args::DoHelp("Don't understand argument '" + a.argument() + "' to '--" + a.long_name() + "'");
    }

    struct UseExistingVisitor
    {
        const ResolveCommandLineResolutionOptions & resolution_options;
        const bool from_set;

        UseExistingVisitor(const ResolveCommandLineResolutionOptions & c, const bool f) :
            resolution_options(c),
            from_set(f)
        {
        }

        std::pair<UseExisting, bool> visit(const DependencyReason &) const
        {
            return std::make_pair(use_existing_from_cmdline(resolution_options.a_keep, false), false);
        }

        std::pair<UseExisting, bool> visit(const TargetReason &) const
        {
            return std::make_pair(use_existing_from_cmdline(resolution_options.a_keep_targets, from_set), false);
        }

        std::pair<UseExisting, bool> visit(const DependentReason &) const
        {
            return std::make_pair(ue_if_possible, false);
        }

        std::pair<UseExisting, bool> visit(const WasUsedByReason &) const
        {
            return std::make_pair(ue_if_possible, false);
        }

        std::pair<UseExisting, bool> visit(const PresetReason &) const
        {
            return std::make_pair(ue_if_possible, false);
        }

        std::pair<UseExisting, bool> visit(const ViaBinaryReason &) const
        {
            return std::make_pair(ue_if_possible, false);
        }

        std::pair<UseExisting, bool> visit(const SetReason & r) const
        {
            UseExistingVisitor v(resolution_options, true);
            return r.reason_for_set()->accept_returning<std::pair<UseExisting, bool> >(v);
        }

        std::pair<UseExisting, bool> visit(const LikeOtherDestinationTypeReason & r) const
        {
            UseExistingVisitor v(resolution_options, true);
            return r.reason_for_other()->accept_returning<std::pair<UseExisting, bool> >(v);
        }
    };

    bool use_existing_from_withish(
            const Environment * const,
            const QualifiedPackageName & name,
            const PackageDepSpecList & list)
    {
        for (PackageDepSpecList::const_iterator l(list.begin()), l_end(list.end()) ;
                l != l_end ; ++l)
        {
            if (! package_dep_spec_has_properties(*l, make_named_values<PackageDepSpecProperties>(
                            n::has_additional_requirements() = false,
                            n::has_category_name_part() = false,
                            n::has_from_repository() = false,
                            n::has_in_repository() = false,
                            n::has_installable_to_path() = false,
                            n::has_installable_to_repository() = false,
                            n::has_installed_at_path() = false,
                            n::has_package() = true,
                            n::has_package_name_part() = false,
                            n::has_slot_requirement() = false,
                            n::has_tag() = indeterminate,
                            n::has_version_requirements() = false
                            )))
                throw args::DoHelp("'" + stringify(*l) + "' is not a simple cat/pkg");

            if (name == *l->package_ptr())
                return true;
        }

        return false;
    }

    std::pair<UseExisting, bool> use_existing_nothing_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpecList & without,
            const PackageDepSpecList & with,
            const std::shared_ptr<const Resolution> &,
            const PackageDepSpec & spec,
            const std::shared_ptr<const Reason> & reason)
    {
        if (spec.package_ptr())
        {
            if (use_existing_from_withish(env, *spec.package_ptr(), without))
                return std::make_pair(ue_if_possible, true);
            if (use_existing_from_withish(env, *spec.package_ptr(), with))
                return std::make_pair(ue_never, false);
        }

        UseExistingVisitor v(resolution_options, false);
        return reason->accept_returning<std::pair<UseExisting, bool> >(v);
    }

    int reinstall_scm_days(const ResolveCommandLineResolutionOptions & resolution_options)
    {
        if (resolution_options.a_reinstall_scm.argument() == "always")
            return 0;
        else if (resolution_options.a_reinstall_scm.argument() == "daily")
            return 1;
        else if (resolution_options.a_reinstall_scm.argument() == "weekly")
            return 7;
        else if (resolution_options.a_reinstall_scm.argument() == "never")
            return -1;
        else
            throw args::DoHelp("Don't understand argument '" + resolution_options.a_reinstall_scm.argument() + "' to '--"
                    + resolution_options.a_reinstall_scm.long_name() + "'");
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

    bool is_scm_older_than(const std::shared_ptr<const PackageID> & id, const int n)
    {
        if (id->version().is_scm() || is_scm_name(id->name()))
        {
            static Timestamp current_time(Timestamp::now()); /* static to avoid weirdness */
            time_t installed_time(current_time.seconds());
            if (id->installed_time_key())
                installed_time = id->installed_time_key()->value().seconds();

            return (current_time.seconds() - installed_time) > (24 * 60 * 60 * n);
        }
        else
            return false;
    }

    bool installed_is_scm_older_than(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const std::shared_ptr<const Generator> & all_binary_repos_generator,
            const Resolvent & q,
            const int n)
    {
        Context context("When working out whether '" + stringify(q) + "' has installed SCM packages:");

        const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                    make_destination_filtered_generator(env, resolution_options, all_binary_repos_generator,
                        generator::Package(q.package()), q) |
                    make_slot_filter(q)
                    )]);

        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if (is_scm_older_than(*i, n))
                return true;
        }

        return false;
    }

    const std::shared_ptr<Constraints> make_initial_constraints_for(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const std::shared_ptr<const Generator> & all_binary_repos_generator,
            const PackageDepSpecList & without,
            const Resolvent & resolvent)
    {
        const std::shared_ptr<Constraints> result(std::make_shared<Constraints>());

        int n(reinstall_scm_days(resolution_options));
        if ((-1 != n) && installed_is_scm_older_than(env, resolution_options, all_binary_repos_generator, resolvent, n)
                && ! use_existing_from_withish(env, resolvent.package(), without))
        {
            result->add(std::make_shared<Constraint>(make_named_values<Constraint>(
                                n::destination_type() = resolvent.destination_type(),
                                n::nothing_is_fine_too() = true,
                                n::reason() = std::make_shared<PresetReason>("is scm", make_null_shared_ptr()),
                                n::spec() = make_package_dep_spec({ }).package(resolvent.package()),
                                n::untaken() = false,
                                n::use_existing() = ue_only_if_transient
                                )));
        }

        return result;
    }

    const std::shared_ptr<Constraints> initial_constraints_for_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpecList & without,
            const InitialConstraints & initial_constraints,
            const std::shared_ptr<const Generator> & all_binary_repos_generator,
            const Resolvent & resolvent)
    {
        InitialConstraints::const_iterator i(initial_constraints.find(resolvent));
        if (i == initial_constraints.end())
            return make_initial_constraints_for(env, resolution_options, all_binary_repos_generator, without, resolvent);
        else
            return i->second;
    }

    struct IsTargetVisitor
    {
        bool visit(const DependencyReason &) const
        {
            return false;
        }

        bool visit(const DependentReason &) const
        {
            return false;
        }

        bool visit(const WasUsedByReason &) const
        {
            return false;
        }

        bool visit(const PresetReason &) const
        {
            return false;
        }

        bool visit(const ViaBinaryReason &) const
        {
            return false;
        }

        bool visit(const TargetReason &) const
        {
            return true;
        }

        bool visit(const LikeOtherDestinationTypeReason & r) const
        {
            return r.reason_for_other()->accept_returning<bool>(*this);
        }

        bool visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<bool>(*this);
        }
    };

    bool is_target(const std::shared_ptr<const Reason> & reason)
    {
        IsTargetVisitor v;
        return reason->accept_returning<bool>(v);
    }

    const std::shared_ptr<Resolvents>
    get_resolvents_for_fn(const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpec & spec,
            const std::shared_ptr<const SlotName> & maybe_slot,
            const std::shared_ptr<const Reason> & reason,
            const DestinationTypes & extra_dts)
    {
        std::shared_ptr<PackageIDSequence> result_ids(std::make_shared<PackageIDSequence>());
        std::shared_ptr<const PackageID> best;

        const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                    generator::Matches(spec, { mpo_ignore_additional_requirements }) |
                    filter::SupportsAction<InstallAction>() |
                    filter::NotMasked() |
                    (maybe_slot ? Filter(filter::Slot(*maybe_slot)) : Filter(filter::All())))]);

        if (! ids->empty())
            best = *ids->begin();

        const std::shared_ptr<const PackageIDSequence> installed_ids((*env)[selection::BestVersionInEachSlot(
                    generator::Matches(spec, { }) |
                    filter::InstalledAtRoot(FSEntry("/")))]);

        const args::EnumArg & arg(is_target(reason) ? resolution_options.a_target_slots : resolution_options.a_slots);

        if (! best)
            std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
        else if (arg.argument() == "best-or-installed")
        {
            if (indirect_iterator(installed_ids->end()) == std::find(indirect_iterator(installed_ids->begin()),
                        indirect_iterator(installed_ids->end()), *best))
                result_ids->push_back(best);
            else
                std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
        }
        else if (arg.argument() == "installed-or-best")
        {
            if (installed_ids->empty())
                result_ids->push_back(best);
            else
                std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
        }
        else if (arg.argument() == "all")
        {
            if (indirect_iterator(installed_ids->end()) == std::find(indirect_iterator(installed_ids->begin()),
                        indirect_iterator(installed_ids->end()), *best))
                result_ids->push_back(best);
            std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
        }
        else if (arg.argument() == "best")
            result_ids->push_back(best);
        else
            throw args::DoHelp("Don't understand argument '" + arg.argument() + "' to '--"
                    + arg.long_name() + "'");

        std::shared_ptr<Resolvents> result(std::make_shared<Resolvents>());
        for (PackageIDSequence::ConstIterator i(result_ids->begin()), i_end(result_ids->end()) ;
                i != i_end ; ++i)
        {
            DestinationTypes destination_types(get_destination_types_for_fn(env, resolution_options, spec, *i, reason) | extra_dts);
            for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
                if (destination_types[*t])
                    result->push_back(Resolvent(*i, *t));
        }

        return result;
    }

    bool ignore_dep_from(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions &,
            const PackageDepSpecList & no_blockers_from,
            const PackageDepSpecList & no_dependencies_from,
            const std::shared_ptr<const PackageID> & id,
            const bool is_block)
    {
        const PackageDepSpecList & list(is_block ? no_blockers_from : no_dependencies_from);

        for (PackageDepSpecList::const_iterator l(list.begin()), l_end(list.end()) ;
                l != l_end ; ++l)
            if (match_package(*env, *l, *id, { }))
                return true;

        return false;
    }

    struct CareAboutDepFnVisitor
    {
        const Environment * const env;
        const ResolveCommandLineResolutionOptions & resolution_options;
        const PackageDepSpecList & no_blockers_from;
        const PackageDepSpecList & no_dependencies_from;
        const SanitisedDependency dep;

        CareAboutDepFnVisitor(const Environment * const e,
                const ResolveCommandLineResolutionOptions & c,
                const PackageDepSpecList & b,
                const PackageDepSpecList & f,
                const SanitisedDependency & d) :
            env(e),
            resolution_options(c),
            no_blockers_from(b),
            no_dependencies_from(f),
            dep(d)
        {
        }

        bool visit(const ExistingNoChangeDecision & decision) const
        {
            if (ignore_dep_from(env, resolution_options, no_blockers_from, no_dependencies_from,
                        decision.existing_id(), bool(dep.spec().if_block())))
                return false;

            if (! is_enabled_dep(dep))
                return false;

            if (! resolution_options.a_follow_installed_build_dependencies.specified())
                if (is_just_build_dep(dep))
                    return false;
            if (resolution_options.a_no_follow_installed_dependencies.specified())
                if (! is_compiled_against_dep(dep))
                    return false;

            if (is_suggestion(dep) || is_recommendation(dep))
            {
                /* we only take a suggestion or recommendation for an existing
                 * package if it's already met. for now, we ignore suggested
                 * and recommended blocks no matter what. */
                if (dep.spec().if_block())
                    return false;

                const std::shared_ptr<const PackageIDSequence> installed_ids(
                        (*env)[selection::SomeArbitraryVersion(
                            generator::Matches(*dep.spec().if_package(), { }) |
                            filter::InstalledAtRoot(FSEntry("/")))]);
                if (installed_ids->empty())
                    return false;
            }

            return true;
        }

        bool visit(const NothingNoChangeDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "NothingNoChangeDecision shouldn't have deps");
        }

        bool visit(const UnableToMakeDecision &) const
        {
            /* might've gone from a sensible decision to unable later on */
            return false;
        }

        bool visit(const RemoveDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "RemoveDecision shouldn't have deps");
        }

        bool visit(const BreakDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "BreakDecision shouldn't have deps");
        }

        bool visit(const ChangesToMakeDecision & decision) const
        {
            if (ignore_dep_from(env, resolution_options, no_blockers_from,
                        no_dependencies_from, decision.origin_id(), bool(dep.spec().if_block())))
                return false;

            if (is_enabled_dep(dep))
                return true;

            return false;
        }
    };

    SpecInterest interest_in_spec_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpecList & take,
            const PackageDepSpecList & take_from,
            const PackageDepSpecList & ignore,
            const PackageDepSpecList & ignore_from,
            const PackageDepSpecList & no_blockers_from,
            const PackageDepSpecList & no_dependencies_from,
            const std::shared_ptr<const Resolution> & resolution,
            const SanitisedDependency & dep)
    {
        CareAboutDepFnVisitor v(env, resolution_options, no_blockers_from, no_dependencies_from, dep);
        if (resolution->decision()->accept_returning<bool>(v))
        {
            bool suggestion(is_suggestion(dep)), recommendation(is_recommendation(dep));

            if (! (suggestion || recommendation))
                return si_take;

            for (PackageDepSpecList::const_iterator l(take.begin()), l_end(take.end()) ;
                    l != l_end ; ++l)
            {
                PackageDepSpec spec(*dep.spec().if_package());
                if (match_qpns(*env, *l, *spec.package_ptr()))
                    return si_take;
            }

            for (PackageDepSpecList::const_iterator l(take_from.begin()), l_end(take_from.end()) ;
                    l != l_end ; ++l)
            {
                if (match_qpns(*env, *l, resolution->resolvent().package()))
                    return si_take;
            }

            for (PackageDepSpecList::const_iterator l(ignore.begin()), l_end(ignore.end()) ;
                    l != l_end ; ++l)
            {
                PackageDepSpec spec(*dep.spec().if_package());
                if (match_qpns(*env, *l, *spec.package_ptr()))
                    return si_ignore;
            }

            for (PackageDepSpecList::const_iterator l(ignore_from.begin()), l_end(ignore_from.end()) ;
                    l != l_end ; ++l)
            {
                if (match_qpns(*env, *l, resolution->resolvent().package()))
                    return si_ignore;
            }

            if (suggestion)
            {
                if (resolution_options.a_suggestions.argument() == "take")
                    return si_take;
                else if (resolution_options.a_suggestions.argument() == "ignore")
                    return si_ignore;
            }

            if (recommendation)
            {
                if (resolution_options.a_recommendations.argument() == "take")
                    return si_take;
                else if (resolution_options.a_recommendations.argument() == "ignore")
                    return si_ignore;
            }

            /* we also take suggestions and recommendations that have already been installed */
            if (dep.spec().if_package())
            {
                const std::shared_ptr<const PackageIDSequence> installed_ids(
                        (*env)[selection::SomeArbitraryVersion(
                            generator::Matches(*dep.spec().if_package(), { }) |
                            filter::InstalledAtRoot(FSEntry("/")))]);
                if (! installed_ids->empty())
                    return si_take;
            }

            return si_untaken;
        }
        else
            return si_ignore;
    }

    const std::shared_ptr<const Repository>
    find_repository_for_fn(const Environment * const env,
            const ResolveCommandLineResolutionOptions &,
            const std::shared_ptr<const Resolution> & resolution,
            const ChangesToMakeDecision & decision)
    {
        std::shared_ptr<const Repository> result;
        for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
                r_end(env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
        {
            switch (resolution->resolvent().destination_type())
            {
                case dt_install_to_slash:
                    if ((! (*r)->installed_root_key()) || ((*r)->installed_root_key()->value() != FSEntry("/")))
                        continue;
                    break;

                case dt_create_binary:
                    if ((*r)->installed_root_key())
                        continue;
                    break;

                case last_dt:
                    break;
            }

            if ((*r)->destination_interface() &&
                    (*r)->destination_interface()->is_suitable_destination_for(*decision.origin_id()))
            {
                if (result)
                    throw ConfigurationError("For '" + stringify(*decision.origin_id())
                            + "' with destination type " + stringify(resolution->resolvent().destination_type())
                            + ", don't know whether to install to ::" + stringify(result->name())
                            + " or ::" + stringify((*r)->name()));
                else
                    result = *r;
            }
        }

        if (! result)
            throw ConfigurationError("No repository suitable for '" + stringify(*decision.origin_id())
                    + "' with destination type " + stringify(resolution->resolvent().destination_type()) + " has been configured");
        return result;
    }

    Filter make_destination_filter_fn(const Resolvent & resolvent)
    {
        switch (resolvent.destination_type())
        {
            case dt_install_to_slash:
                return filter::InstalledAtRoot(FSEntry("/"));

            case dt_create_binary:
                throw InternalError(PALUDIS_HERE, "no dt_create_binary yet");

            case last_dt:
                break;
        }

        throw InternalError(PALUDIS_HERE, "unhandled dt");
    }

    bool match_any(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::shared_ptr<const PackageID> & i)
    {
        for (PackageDepSpecList::const_iterator l(list.begin()), l_end(list.end()) ;
                l != l_end ; ++l)
            if (match_package(*env, *l, *i, { }))
                return true;

        return false;
    }

    struct AllowedToRemoveVisitor
    {
        bool visit(const DependentReason &) const
        {
            return true;
        }

        bool visit(const TargetReason &) const
        {
            return true;
        }

        bool visit(const DependencyReason &) const
        {
            return false;
        }

        bool visit(const WasUsedByReason &) const
        {
            return true;
        }

        bool visit(const ViaBinaryReason &) const
        {
            return false;
        }

        bool visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<bool>(*this);
        }

        bool visit(const LikeOtherDestinationTypeReason & r) const
        {
            return r.reason_for_other()->accept_returning<bool>(*this);
        }

        bool visit(const PresetReason &) const
        {
            return false;
        }
    };

    bool allowed_to_remove_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::shared_ptr<const Resolution> & resolution,
            const std::shared_ptr<const PackageID> & i)
    {
        for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                c_end(resolution->constraints()->end()) ;
                c != c_end ; ++c)
            if ((*c)->reason()->accept_returning<bool>(AllowedToRemoveVisitor()))
                return true;

        return match_any(env, list, i);
    }

    bool remove_if_dependent_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::shared_ptr<const PackageID> & i)
    {
        return match_any(env, list, i);
    }

    bool prefer_or_avoid_one(const Environment * const, const QualifiedPackageName & q, const PackageDepSpec & s)
    {
        Context context("When working out whether we favour or avoid '" + stringify(q) + "' due to '"
                + stringify(s) + "':");

        if (! package_dep_spec_has_properties(s, make_named_values<PackageDepSpecProperties>(
                        n::has_additional_requirements() = false,
                        n::has_category_name_part() = false,
                        n::has_from_repository() = false,
                        n::has_in_repository() = false,
                        n::has_installable_to_path() = false,
                        n::has_installable_to_repository() = false,
                        n::has_installed_at_path() = false,
                        n::has_package() = true,
                        n::has_package_name_part() = false,
                        n::has_slot_requirement() = false,
                        n::has_tag() = indeterminate,
                        n::has_version_requirements() = false
                        )))
            throw args::DoHelp("'" + stringify(s) + "' is not a simple cat/pkg");
        return *s.package_ptr() == q;
    }

    Tribool prefer_or_avoid_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions &,
            const PackageDepSpecList & favour,
            const PackageDepSpecList & avoid,
            const QualifiedPackageName & q)
    {
        if (favour.end() != std::find_if(favour.begin(), favour.end(),
                    std::bind(&prefer_or_avoid_one, env, q, std::placeholders::_1)))
            return true;
        if (avoid.end() != std::find_if(avoid.begin(), avoid.end(),
                    std::bind(&prefer_or_avoid_one, env, q, std::placeholders::_1)))
            return false;
        return indeterminate;
    }

    struct ChosenIDVisitor
    {
        const std::shared_ptr<const PackageID> visit(const ChangesToMakeDecision & decision) const
        {
            return decision.origin_id();
        }

        const std::shared_ptr<const PackageID> visit(const BreakDecision & decision) const
        {
            return decision.existing_id();
        }

        const std::shared_ptr<const PackageID> visit(const ExistingNoChangeDecision & decision) const
        {
            return decision.existing_id();
        }

        const std::shared_ptr<const PackageID> visit(const NothingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const PackageID> visit(const RemoveDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const PackageID> visit(const UnableToMakeDecision &) const
        {
            return make_null_shared_ptr();
        }
    };

    Tribool order_early_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions &,
            const PackageDepSpecList & early,
            const PackageDepSpecList & late,
            const std::shared_ptr<const Resolution> & r)
    {
        const std::shared_ptr<const PackageID> id(r->decision()->accept_returning<std::shared_ptr<const PackageID> >(
                    ChosenIDVisitor()));
        if (id)
        {
            if (match_any(env, early, id))
                return true;
            if (match_any(env, late, id))
                return false;
        }

        return indeterminate;
    }

    struct ConfirmFnVisitor
    {
        const Environment * const env;
        const ResolveCommandLineResolutionOptions & resolution_options;
        const PackageDepSpecList & permit_downgrade;
        const PackageDepSpecList & permit_old_version;
        const PackageDepSpecList & allowed_to_break_specs;
        const bool allowed_to_break_system;
        const std::shared_ptr<const PackageID> id;

        ConfirmFnVisitor(const Environment * const e,
                const ResolveCommandLineResolutionOptions & r,
                const PackageDepSpecList & d,
                const PackageDepSpecList & o,
                const PackageDepSpecList & a,
                const bool s,
                const std::shared_ptr<const PackageID> & i) :
            env(e),
            resolution_options(r),
            permit_downgrade(d),
            permit_old_version(o),
            allowed_to_break_specs(a),
            allowed_to_break_system(s),
            id(i)
        {
        }

        bool visit(const DowngradeConfirmation &) const
        {
            if (id)
                for (PackageDepSpecList::const_iterator l(permit_downgrade.begin()), l_end(permit_downgrade.end()) ;
                        l != l_end ; ++l)
                {
                    if (match_package(*env, *l, *id, { }))
                        return true;
                }

            return false;
        }

        bool visit(const NotBestConfirmation &) const
        {
            if (id)
                for (PackageDepSpecList::const_iterator l(permit_old_version.begin()), l_end(permit_old_version.end()) ;
                        l != l_end ; ++l)
                {
                    if (match_package(*env, *l, *id, { }))
                        return true;
                }

            return false;
        }

        bool visit(const BreakConfirmation &) const
        {
            if (id)
                for (PackageDepSpecList::const_iterator l(allowed_to_break_specs.begin()), l_end(allowed_to_break_specs.end()) ;
                        l != l_end ; ++l)
                {
                    if (match_package(*env, *l, *id, { }))
                        return true;
                }

            return false;
        }

        bool visit(const RemoveSystemPackageConfirmation &) const
        {
            return allowed_to_break_system;
        }

        bool visit(const MaskedConfirmation &) const
        {
            return false;
        }
    };

    bool confirm_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const PackageDepSpecList & permit_downgrade,
            const PackageDepSpecList & permit_old_version,
            const PackageDepSpecList & allowed_to_break_specs,
            const bool allowed_to_break_system,
            const std::shared_ptr<const Resolution> & r,
            const std::shared_ptr<const RequiredConfirmation> & c)
    {
        return c->accept_returning<bool>(ConfirmFnVisitor(env, resolution_options, permit_downgrade, permit_old_version,
                    allowed_to_break_specs, allowed_to_break_system,
                    r->decision()->accept_returning<std::shared_ptr<const PackageID> >(ChosenIDVisitor())
                    ));
    }

    const std::shared_ptr<ConstraintSequence> get_constraints_for_dependent_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::shared_ptr<const Resolution> &,
            const std::shared_ptr<const PackageID> & id,
            const std::shared_ptr<const ChangeByResolventSequence> & ids)
    {
        const std::shared_ptr<ConstraintSequence> result(std::make_shared<ConstraintSequence>());

        std::shared_ptr<PackageDepSpec> spec;
        if (match_any(env, list, id))
            spec = make_shared_copy(id->uniquely_identifying_spec());
        else
        {
            PartiallyMadePackageDepSpec partial_spec({ });
            partial_spec.package(id->name());
            if (id->slot_key())
                partial_spec.slot_requirement(std::make_shared<ELikeSlotExactRequirement>(
                                id->slot_key()->value(), false));
            spec = std::make_shared<PackageDepSpec>(partial_spec);
        }

        for (ChangeByResolventSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            const std::shared_ptr<DependentReason> reason(std::make_shared<DependentReason>(*i));

            result->push_back(std::make_shared<Constraint>(make_named_values<Constraint>(
                                n::destination_type() = dt_install_to_slash,
                                n::nothing_is_fine_too() = true,
                                n::reason() = reason,
                                n::spec() = BlockDepSpec("!" + stringify(*spec), *spec, false),
                                n::untaken() = false,
                                n::use_existing() = ue_if_possible
                                )));
        }

        return result;
    }

    const std::shared_ptr<ConstraintSequence> get_constraints_for_purge_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::shared_ptr<const Resolution> &,
            const std::shared_ptr<const PackageID> & id,
            const std::shared_ptr<const ChangeByResolventSequence> & ids)
    {
        const std::shared_ptr<ConstraintSequence> result(std::make_shared<ConstraintSequence>());

        PartiallyMadePackageDepSpec partial_spec({ });
        partial_spec.package(id->name());
        if (id->slot_key())
            partial_spec.slot_requirement(std::make_shared<ELikeSlotExactRequirement>(
                            id->slot_key()->value(), false));
        PackageDepSpec spec(partial_spec);

        const std::shared_ptr<WasUsedByReason> reason(std::make_shared<WasUsedByReason>(ids));

        result->push_back(std::make_shared<Constraint>(make_named_values<Constraint>(
                            n::destination_type() = dt_install_to_slash,
                            n::nothing_is_fine_too() = true,
                            n::reason() = reason,
                            n::spec() = BlockDepSpec("!" + stringify(spec), spec, false),
                            n::untaken() = ! match_any(env, list, id),
                            n::use_existing() = ue_if_possible
                            )));

        return result;
    }

    const std::shared_ptr<ConstraintSequence> get_constraints_for_via_binary_fn(
            const Environment * const,
            const std::shared_ptr<const Resolution> & resolution,
            const std::shared_ptr<const Resolution> & other_resolution)
    {
        const std::shared_ptr<ConstraintSequence> result(std::make_shared<ConstraintSequence>());

        PartiallyMadePackageDepSpec partial_spec({ });
        partial_spec.package(resolution->resolvent().package());
        PackageDepSpec spec(partial_spec);

        const std::shared_ptr<ViaBinaryReason> reason(std::make_shared<ViaBinaryReason>(other_resolution->resolvent()));

        result->push_back(std::make_shared<Constraint>(make_named_values<Constraint>(
                            n::destination_type() = resolution->resolvent().destination_type(),
                            n::nothing_is_fine_too() = false,
                            n::reason() = reason,
                            n::spec() = spec,
                            n::untaken() = true,
                            n::use_existing() = ue_never
                            )));

        return result;
    }

    bool can_use_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::shared_ptr<const PackageID> & id)
    {
        return ! match_any(env, list, id);
    }

    bool always_via_binary_fn(
            const Environment * const env,
            const PackageDepSpecList & list,
            const std::shared_ptr<const Resolution> & resolution)
    {
        const ChangesToMakeDecision * changes_decision(simple_visitor_cast<const ChangesToMakeDecision>(*resolution->decision()));
        if (! changes_decision)
            return false;

        return can_make_binary_for(changes_decision->origin_id()) && match_any(env, list, changes_decision->origin_id());
    }

    void serialise_resolved(StringListStream & ser_stream, const Resolved & resolved)
    {
        Serialiser ser(ser_stream);
        resolved.serialise(ser);
        ser_stream.nothing_more_to_write();
    }

    int display_resolution(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const ResolveCommandLineResolutionOptions &,
            const ResolveCommandLineDisplayOptions & display_options,
            const ResolveCommandLineProgramOptions & program_options,
            const std::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
            const std::shared_ptr<const Sequence<std::pair<std::string, std::string> > > & targets)
    {
        Context context("When displaying chosen resolution:");

        StringListStream ser_stream;
        Thread ser_thread(std::bind(&serialise_resolved,
                    std::ref(ser_stream),
                    std::cref(*resolved)));

        std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());

        for (args::ArgsSection::GroupsConstIterator g(display_options.begin()), g_end(display_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                {
                    const std::shared_ptr<const Sequence<std::string> > f((*o)->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
        }

        for (Sequence<std::pair<std::string, std::string> >::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
            args->push_back(p->first);

        if (program_options.a_display_resolution_program.specified())
        {
            std::string command(program_options.a_display_resolution_program.argument());

            if (keys_if_import)
                for (Map<std::string, std::string>::ConstIterator k(keys_if_import->begin()),
                        k_end(keys_if_import->end()) ;
                        k != k_end ; ++k)
                {
                    args->push_back("--unpackaged-repository-params");
                    args->push_back(k->first + "=" + k->second);
                }

            for (Sequence<std::string>::ConstIterator a(args->begin()), a_end(args->end()) ;
                    a != a_end ; ++a)
                command = command + " " + args::escape(*a);

            paludis::Command cmd(command);
            cmd
                .with_input_stream(&ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

            return run_command(cmd);
        }
        else
            return DisplayResolutionCommand().run(env, args, resolved);
    }

    void serialise_job_lists(StringListStream & ser_stream, const JobLists & job_lists)
    {
        Serialiser ser(ser_stream);
        job_lists.serialise(ser);
        ser_stream.nothing_more_to_write();
    }

    int perform_resolution(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const ResolveCommandLineExecutionOptions & execution_options,
            const ResolveCommandLineProgramOptions & program_options,
            const std::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
            const std::shared_ptr<const Sequence<std::pair<std::string, std::string> > > & targets,
            const std::shared_ptr<const Sequence<std::string> > & world_specs,
            const bool is_set)
    {
        Context context("When performing chosen resolution:");

        std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());

        if (is_set)
            args->push_back("--set");

        for (args::ArgsSection::GroupsConstIterator g(program_options.begin()), g_end(program_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                {
                    const std::shared_ptr<const Sequence<std::string> > f((*o)->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
        }

        for (args::ArgsSection::GroupsConstIterator g(execution_options.begin()), g_end(execution_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                {
                    const std::shared_ptr<const Sequence<std::string> > f((*o)->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
        }

        if (! resolution_options.a_execute.specified())
            args->push_back("--pretend");

        for (Sequence<std::string>::ConstIterator p(world_specs->begin()), p_end(world_specs->end()) ;
                p != p_end ; ++p)
        {
            args->push_back("--world-specs");
            args->push_back(*p);
        }

        for (Sequence<std::pair<std::string, std::string> >::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
            args->push_back(p->first);

        if (program_options.a_execute_resolution_program.specified() || resolution_options.a_execute.specified())
        {
            /* backgrounding this barfs with become_command. working out why could
             * be a fun exercise for someone with way too much time on their hands.
             * */
            StringListStream ser_stream;
            serialise_job_lists(ser_stream, *resolved->job_lists());

            std::string command;
            if (program_options.a_execute_resolution_program.specified())
                command = program_options.a_execute_resolution_program.argument();
            else
                command = "$CAVE execute-resolution";

            if (keys_if_import)
                for (Map<std::string, std::string>::ConstIterator k(keys_if_import->begin()),
                        k_end(keys_if_import->end()) ;
                        k != k_end ; ++k)
                {
                    args->push_back("--unpackaged-repository-params");
                    args->push_back(k->first + "=" + k->second);
                }

            for (Sequence<std::string>::ConstIterator a(args->begin()), a_end(args->end()) ;
                    a != a_end ; ++a)
                command = command + " " + args::escape(*a);

            paludis::Command cmd(command);
            cmd
                .with_input_stream(&ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

            become_command(cmd);
        }
        else
            return ExecuteResolutionCommand().run(env, args, resolved->job_lists());
    }

    struct KindNameVisitor
    {
        const std::string visit(const RemoveDecision &) const
        {
            return "remove_decision";
        }

        const std::string visit(const BreakDecision &) const
        {
            return "break_decision";
        }

        const std::string visit(const UnableToMakeDecision &) const
        {
            return "unable_to_make_decision";
        }

        const std::string visit(const NothingNoChangeDecision &) const
        {
            return "nothing_no_change";
        }

        const std::string visit(const ExistingNoChangeDecision &) const
        {
            return "existing_no_change";
        }

        const std::string visit(const ChangesToMakeDecision &) const
        {
            return "changes_to_make";
        }
    };

    std::string stringify_change_by_resolvent(const ChangeByResolvent & r)
    {
        return stringify(*r.package_id());
    }

    struct ShortReasonName
    {
        const std::string visit(const DependencyReason & r) const
        {
            return "from " + stringify(*r.from_id()) + " dependency " + (r.sanitised_dependency().spec().if_package() ?
                    stringify(*r.sanitised_dependency().spec().if_package()) : stringify(*r.sanitised_dependency().spec().if_block()));
        }

        const std::string visit(const WasUsedByReason & r) const
        {
            if (r.ids_and_resolvents_being_removed()->empty())
                return "from was unused";
            else
                return "from was used by " + join(r.ids_and_resolvents_being_removed()->begin(),
                        r.ids_and_resolvents_being_removed()->end(), ", ", stringify_change_by_resolvent);
        }

        const std::string visit(const DependentReason & r) const
        {
            return "from dependent " + stringify(*r.id_and_resolvent_being_removed().package_id());
        }

        const std::string visit(const TargetReason & r) const
        {
            return "from target" + (r.extra_information().empty() ? "" : " (" + r.extra_information() + ")");
        }

        const std::string visit(const ViaBinaryReason &) const
        {
            return "from via binary";
        }

        const std::string visit(const PresetReason & r) const
        {
            std::string result("from preset");
            if (! r.maybe_explanation().empty())
                result = result + " (" + r.maybe_explanation() + ")";
            if (r.maybe_reason_for_preset())
                result = result + " (" + r.maybe_reason_for_preset()->accept_returning<std::string>(*this) + ")";
            return result;
        }

        const std::string visit(const SetReason & r) const
        {
            return "from " + stringify(r.set_name()) + " (" + r.reason_for_set()->accept_returning<std::string>(*this) + ")";
        }

        const std::string visit(const LikeOtherDestinationTypeReason & r) const
        {
            return "from being like " + stringify(r.other_resolvent())
                + " (" + r.reason_for_other()->accept_returning<std::string>(*this) + ")";
        }
    };

    void display_restarts_if_requested(const std::list<SuggestRestart> & restarts,
            const ResolveCommandLineResolutionOptions & resolution_options)
    {
        if (! resolution_options.a_dump_restarts.specified())
            return;

        std::cout << "Dumping restarts:" << std::endl << std::endl;

        for (std::list<SuggestRestart>::const_iterator r(restarts.begin()), r_end(restarts.end()) ;
                r != r_end ; ++r)
        {
            std::cout << "* " << r->resolvent() << std::endl;

            std::cout << "    Had decided upon ";
            const std::shared_ptr<const PackageID> id(r->previous_decision()->accept_returning<
                    std::shared_ptr<const PackageID> >(ChosenIDVisitor()));
            if (id)
                std::cout << *id;
            else
                std::cout << r->previous_decision()->accept_returning<std::string>(KindNameVisitor());
            std::cout << std::endl;

            std::cout << "    Which did not satisfy " << r->problematic_constraint()->spec()
                << ", use existing " << r->problematic_constraint()->use_existing();
            if (r->problematic_constraint()->nothing_is_fine_too())
                std::cout << ", nothing is fine too";
            std::cout << " " << r->problematic_constraint()->reason()->accept_returning<std::string>(ShortReasonName());
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }
}

int
paludis::cave::resolve_common(
        const std::shared_ptr<Environment> & env,
        const ResolveCommandLineResolutionOptions & resolution_options,
        const ResolveCommandLineExecutionOptions & execution_options,
        const ResolveCommandLineDisplayOptions & display_options,
        const ResolveCommandLineProgramOptions & program_options,
        const std::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
        const std::shared_ptr<const Sequence<std::pair<std::string, std::string> > > & targets_if_not_purge,
        const std::shared_ptr<const Sequence<std::string> > & world_specs_if_not_auto,
        const bool purge)
{
    int retcode(0);

    InitialConstraints initial_constraints;
    PackageDepSpecList allowed_to_remove_specs, allowed_to_break_specs, remove_if_dependent_specs,
                       less_restrictive_remove_blockers_specs, purge_specs, with, without,
                       permit_old_version, permit_downgrade, take, take_from, ignore, ignore_from,
                       favour, avoid, early, late, no_dependencies_from, no_blockers_from, not_usable_specs,
                       via_binary_specs;
    bool allowed_to_break_system(false);

    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_uninstall.begin_args()),
            i_end(resolution_options.a_permit_uninstall.end_args()) ;
            i != i_end ; ++i)
        allowed_to_remove_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_uninstalls_may_break.begin_args()),
            i_end(resolution_options.a_uninstalls_may_break.end_args()) ;
            i != i_end ; ++i)
        if (*i == "system")
            allowed_to_break_system = true;
        else
            allowed_to_break_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                        { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_remove_if_dependent.begin_args()),
            i_end(resolution_options.a_remove_if_dependent.end_args()) ;
            i != i_end ; ++i)
        remove_if_dependent_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_less_restrictive_remove_blockers.begin_args()),
            i_end(resolution_options.a_less_restrictive_remove_blockers.end_args()) ;
            i != i_end ; ++i)
        less_restrictive_remove_blockers_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_purge.begin_args()),
            i_end(resolution_options.a_purge.end_args()) ;
            i != i_end ; ++i)
        purge_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_without.begin_args()),
            i_end(resolution_options.a_without.end_args()) ;
            i != i_end ; ++i)
        without.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_with.begin_args()),
            i_end(resolution_options.a_with.end_args()) ;
            i != i_end ; ++i)
        with.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_take.begin_args()),
            i_end(resolution_options.a_take.end_args()) ;
            i != i_end ; ++i)
        take.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_take_from.begin_args()),
            i_end(resolution_options.a_take_from.end_args()) ;
            i != i_end ; ++i)
        take_from.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_ignore.begin_args()),
            i_end(resolution_options.a_ignore.end_args()) ;
            i != i_end ; ++i)
        ignore.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_ignore_from.begin_args()),
            i_end(resolution_options.a_ignore_from.end_args()) ;
            i != i_end ; ++i)
        ignore_from.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_favour.begin_args()),
            i_end(resolution_options.a_favour.end_args()) ;
            i != i_end ; ++i)
        favour.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_avoid.begin_args()),
            i_end(resolution_options.a_avoid.end_args()) ;
            i != i_end ; ++i)
        avoid.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_early.begin_args()),
            i_end(resolution_options.a_early.end_args()) ;
            i != i_end ; ++i)
        early.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_late.begin_args()),
            i_end(resolution_options.a_late.end_args()) ;
            i != i_end ; ++i)
        late.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_no_dependencies_from.begin_args()),
            i_end(resolution_options.a_no_dependencies_from.end_args()) ;
            i != i_end ; ++i)
        no_dependencies_from.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_no_blockers_from.begin_args()),
            i_end(resolution_options.a_no_blockers_from.end_args()) ;
            i != i_end ; ++i)
        no_blockers_from.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_downgrade.begin_args()),
            i_end(resolution_options.a_permit_downgrade.end_args()) ;
            i != i_end ; ++i)
        permit_downgrade.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_old_version.begin_args()),
            i_end(resolution_options.a_permit_old_version.end_args()) ;
            i != i_end ; ++i)
        permit_old_version.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_not_usable.begin_args()),
            i_end(resolution_options.a_not_usable.end_args()) ;
            i != i_end ; ++i)
        not_usable_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));

#ifdef ENABLE_PBINS
    for (args::StringSetArg::ConstIterator i(resolution_options.a_via_binary.begin_args()),
            i_end(resolution_options.a_via_binary.end_args()) ;
            i != i_end ; ++i)
        via_binary_specs.push_back(parse_user_package_dep_spec(*i, env.get(),
                    { updso_allow_wildcards }));
#endif

    std::shared_ptr<Generator> all_binary_repos_generator, not_binary_repos_generator;
    for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
            r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
        if (! (*r)->installed_root_key())
        {
            if ((*r)->destination_interface())
            {
                if (all_binary_repos_generator)
                    all_binary_repos_generator = std::make_shared<generator::Union>(*all_binary_repos_generator,
                                generator::InRepository((*r)->name()));
                else
                    all_binary_repos_generator = std::make_shared<generator::InRepository>((*r)->name());
            }
            else
            {
                if (not_binary_repos_generator)
                    not_binary_repos_generator = std::make_shared<generator::Union>(*not_binary_repos_generator,
                                generator::InRepository((*r)->name()));
                else
                    not_binary_repos_generator = std::make_shared<generator::InRepository>((*r)->name());
            }
        }

    if (! all_binary_repos_generator)
        all_binary_repos_generator = std::make_shared<generator::Nothing>();

    for (args::StringSetArg::ConstIterator i(resolution_options.a_preset.begin_args()),
            i_end(resolution_options.a_preset.end_args()) ;
            i != i_end ; ++i)
    {
        const std::shared_ptr<const Reason> reason(std::make_shared<PresetReason>("preset", make_null_shared_ptr()));
        PackageDepSpec spec(parse_user_package_dep_spec(*i, env.get(), { }));
        DestinationTypes all_dts;
        for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
            all_dts += *t;
        const std::shared_ptr<const Resolvents> resolvents(get_resolvents_for_fn(
                    env.get(), resolution_options, spec, make_null_shared_ptr(), reason,
                    all_dts));

        if (resolvents->empty())
            throw args::DoHelp("Preset '" + *i + "' has no resolvents");

        for (Resolvents::ConstIterator r(resolvents->begin()), r_end(resolvents->end()) ;
                r != r_end ; ++r)
        {
            const std::shared_ptr<Constraint> constraint(std::make_shared<Constraint>(make_named_values<Constraint>(
                            n::destination_type() = r->destination_type(),
                            n::nothing_is_fine_too() = true,
                            n::reason() = reason,
                            n::spec() = spec,
                            n::untaken() = false,
                            n::use_existing() = ue_if_possible
                            )));
            initial_constraints.insert(std::make_pair(*r, make_initial_constraints_for(
                            env.get(), resolution_options, all_binary_repos_generator, without, *r))).first->second->add(
                    constraint);
        }
    }

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    using std::placeholders::_4;

    ResolverFunctions resolver_functions(make_named_values<ResolverFunctions>(
                n::allowed_to_remove_fn() = std::bind(&allowed_to_remove_fn,
                    env.get(), std::cref(allowed_to_remove_specs), _1, _2),
                n::always_via_binary_fn() = std::bind(&always_via_binary_fn,
                    env.get(), std::cref(via_binary_specs), _1),
                n::can_use_fn() = std::bind(&can_use_fn,
                    env.get(), std::cref(not_usable_specs), _1),
                n::confirm_fn() = std::bind(&confirm_fn,
                    env.get(), std::cref(resolution_options), std::cref(permit_downgrade),
                    std::cref(permit_old_version), std::cref(allowed_to_break_specs),
                    allowed_to_break_system, _1, _2),
                n::find_repository_for_fn() = std::bind(&find_repository_for_fn,
                    env.get(), std::cref(resolution_options), _1, _2),
                n::get_constraints_for_dependent_fn() = std::bind(&get_constraints_for_dependent_fn,
                    env.get(), std::cref(less_restrictive_remove_blockers_specs), _1, _2, _3),
                n::get_constraints_for_purge_fn() = std::bind(&get_constraints_for_purge_fn,
                    env.get(), std::cref(purge_specs), _1, _2, _3),
                n::get_constraints_for_via_binary_fn() = std::bind(&get_constraints_for_via_binary_fn,
                    env.get(), _1, _2),
                n::get_destination_types_for_fn() = std::bind(&get_destination_types_for_fn,
                    env.get(), std::cref(resolution_options), _1, _2, _3),
                n::get_initial_constraints_for_fn() = std::bind(&initial_constraints_for_fn,
                    env.get(), std::cref(resolution_options), std::cref(without),
                    std::cref(initial_constraints), all_binary_repos_generator, _1),
                n::get_resolvents_for_fn() = std::bind(&get_resolvents_for_fn,
                    env.get(), std::cref(resolution_options), _1, _2, _3, DestinationTypes()),
                n::get_use_existing_nothing_fn() = std::bind(&use_existing_nothing_fn,
                    env.get(), std::cref(resolution_options), std::cref(without), std::cref(with), _1, _2, _3),
                n::interest_in_spec_fn() = std::bind(&interest_in_spec_fn,
                    env.get(), std::cref(resolution_options), std::cref(take), std::cref(take_from),
                    std::cref(ignore), std::cref(ignore_from), std::cref(no_blockers_from),
                    std::cref(no_dependencies_from), _1, _2),
                n::make_destination_filtered_generator_fn() = std::bind(&make_destination_filtered_generator_with_resolution,
                    env.get(), std::cref(resolution_options), all_binary_repos_generator, _1, _2),
                n::make_origin_filtered_generator_fn() = std::bind(&make_origin_filtered_generator,
                    env.get(), std::cref(resolution_options), not_binary_repos_generator, _1, _2),
                n::make_unmaskable_filter_fn() = std::bind(&make_unmaskable_filter_fn,
                        env.get(), std::cref(resolution_options), _1),
                n::order_early_fn() = std::bind(&order_early_fn,
                    env.get(), std::cref(resolution_options), std::cref(early), std::cref(late), _1),
                n::prefer_or_avoid_fn() = std::bind(&prefer_or_avoid_fn,
                    env.get(), std::cref(resolution_options), std::cref(favour), std::cref(avoid), _1),
                n::remove_if_dependent_fn() = std::bind(&remove_if_dependent_fn,
                    env.get(), std::cref(remove_if_dependent_specs), _1)
                ));

    ScopedSelectionCache selection_cache(env.get());
    std::shared_ptr<Resolver> resolver(std::make_shared<Resolver>(env.get(), resolver_functions));
    bool is_set(false);
    std::shared_ptr<const Sequence<std::string> > targets_cleaned_up;
    std::list<SuggestRestart> restarts;

    try
    {
        {
            DisplayCallback display_callback("Resolving: ");
            ScopedNotifierCallback display_callback_holder(env.get(),
                    NotifierCallbackFunction(std::cref(display_callback)));

            while (true)
            {
                try
                {
                    if (purge)
                    {
                        resolver->purge();
                        targets_cleaned_up = std::make_shared<Sequence<std::string>>();
                    } else
                        targets_cleaned_up = add_resolver_targets(env, resolver, resolution_options, targets_if_not_purge, is_set);
                    resolver->resolve();
                    break;
                }
                catch (const SuggestRestart & e)
                {
                    restarts.push_back(e);
                    display_callback(ResolverRestart());
                    initial_constraints.insert(std::make_pair(e.resolvent(), make_initial_constraints_for(
                                    env.get(), resolution_options, all_binary_repos_generator, without, e.resolvent()))).first->second->add(
                            e.suggested_preset());
                    resolver = std::make_shared<Resolver>(env.get(), resolver_functions);

                    if (restarts.size() > 9000)
                        throw InternalError(PALUDIS_HERE, "Restarted over nine thousand times. Something's "
                                "probably gone horribly wrong. Consider using --dump-restarts and having "
                                "a look.");
                }
            }
        }

        if (! restarts.empty())
            display_restarts_if_requested(restarts, resolution_options);

        dump_if_requested(env, resolver, resolution_options);

        retcode |= display_resolution(env, resolver->resolved(), resolution_options,
                display_options, program_options, keys_if_import,
                purge ? std::make_shared<const Sequence<std::pair<std::string, std::string> > >() : targets_if_not_purge);

        if (! resolver->resolved()->taken_unable_to_make_decisions()->empty())
            retcode |= 1;

        if (! resolver->resolved()->taken_unconfirmed_decisions()->empty())
            retcode |= 2;

        if (! resolver->resolved()->taken_unorderable_decisions()->empty())
            retcode |= 4;

        if (0 == retcode)
            return perform_resolution(env, resolver->resolved(), resolution_options,
                    execution_options, program_options, keys_if_import,
                    purge ? std::make_shared<const Sequence<std::pair<std::string, std::string> > >() : targets_if_not_purge,
                    world_specs_if_not_auto ? world_specs_if_not_auto : targets_cleaned_up,
                    is_set);
    }
    catch (...)
    {
        if (! restarts.empty())
            display_restarts_if_requested(restarts, resolution_options);

        dump_if_requested(env, resolver, resolution_options);
        throw;
    }

    return retcode | 1;
}

template class Sequence<std::pair<std::string, std::string> >;
template class WrappedForwardIterator<Sequence<std::pair<std::string, std::string> >::ConstIteratorTag,
         const std::pair<std::string, std::string> >;

