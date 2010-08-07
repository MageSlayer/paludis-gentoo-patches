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
#include <paludis/util/return_literal_function.hh>

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

#include <paludis/resolver/allow_choice_changes_helper.hh>
#include <paludis/resolver/allowed_to_remove_helper.hh>
#include <paludis/resolver/always_via_binary_helper.hh>
#include <paludis/resolver/can_use_helper.hh>
#include <paludis/resolver/confirm_helper.hh>
#include <paludis/resolver/find_repository_for_helper.hh>
#include <paludis/resolver/get_constraints_for_dependent_helper.hh>
#include <paludis/resolver/get_constraints_for_purge_helper.hh>
#include <paludis/resolver/get_constraints_for_via_binary_helper.hh>
#include <paludis/resolver/get_destination_types_for_error_helper.hh>
#include <paludis/resolver/remove_if_dependent_helper.hh>

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
#include <paludis/package_id.hh>
#include <paludis/partially_made_package_dep_spec.hh>

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

    bool can_chroot(const std::shared_ptr<const PackageID> & id)
    {
        if (! id->behaviours_key())
            return true;
        return id->behaviours_key()->value()->end() == id->behaviours_key()->value()->find("unchrootable");
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

    struct BinaryableFilterHandler :
        AllFilterHandlerBase
    {
        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & id) const
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                    i != i_end ; ++i)
                if (can_make_binary_for(*i))
                    result->insert(*i);

            return result;
        }

        virtual std::string as_string() const
        {
            return "binaryable";
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

    struct BinaryableFilter :
        Filter
    {
        BinaryableFilter() :
            Filter(std::make_shared<BinaryableFilterHandler>())
        {
        }
    };

    struct DestinationTypesFinder
    {
        const Environment * const env;
        const ResolveCommandLineResolutionOptions & resolution_options;
        const std::shared_ptr<const PackageID> package_id_unless_error;

        DestinationTypesFinder(
                const Environment * const e,
                const ResolveCommandLineResolutionOptions & c,
                const std::shared_ptr<const PackageID> & i) :
            env(e),
            resolution_options(c),
            package_id_unless_error(i)
        {
        }

        DestinationTypes visit(const TargetReason &) const
        {
            if (resolution_options.a_make.argument() == "binaries")
                return DestinationTypes() + dt_create_binary;
            else if (resolution_options.a_make.argument() == "install")
                return DestinationTypes() + dt_install_to_slash;
            else if (resolution_options.a_make.argument() == "chroot")
                return DestinationTypes() + dt_install_to_chroot;
            else
                throw args::DoHelp("Don't understand argument '" + resolution_options.a_make.argument() + "' to '--"
                        + resolution_options.a_make.long_name() + "'");
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

                if (binary_if_possible && package_id_unless_error && can_make_binary_for(package_id_unless_error))
                    extras += dt_create_binary;
            }
            else if (resolution_options.a_make.argument() == "chroot")
            {
                bool chroot_if_possible(false);
                if (resolution_options.a_make_dependencies.argument() == "auto" ||
                        resolution_options.a_make_dependencies.argument() == "all")
                    chroot_if_possible = true;
                else if (resolution_options.a_make_dependencies.argument() == "runtime")
                {
                    if (is_run_or_post_dep(dep.sanitised_dependency()))
                        chroot_if_possible = true;
                }

                if (chroot_if_possible && package_id_unless_error && can_chroot(package_id_unless_error))
                    extras += dt_install_to_chroot;
            }

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
            const std::shared_ptr<const PackageID> & id_unless_error,
            const std::shared_ptr<const Reason> & reason)
    {
        DestinationTypesFinder f(env, resolution_options, id_unless_error);
        return reason->accept_returning<DestinationTypes>(f);
    }

    FilteredGenerator make_destination_filtered_generator(
            const Environment * const,
            const ResolveCommandLineResolutionOptions &,
            const std::shared_ptr<const Generator> & binary_destinations,
            const Generator & g,
            const Resolvent & r)
    {
        switch (r.destination_type())
        {
            case dt_install_to_slash:
                return g | filter::InstalledAtSlash();

            case dt_install_to_chroot:
                return g | filter::InstalledAtNotSlash();

            case dt_create_binary:
                if (binary_destinations)
                    return g & *binary_destinations;
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
            const std::shared_ptr<const Generator> & binary_destinations,
            const Generator & g,
            const std::shared_ptr<const Resolution> & r)
    {
        return make_destination_filtered_generator(env, resolution_options, binary_destinations, g, r->resolvent());
    }

    FilteredGenerator make_origin_filtered_generator(
            const Environment * const,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const Generator & g,
            const std::shared_ptr<const Resolution> &)
    {
        if (resolution_options.a_make.argument() == "binaries")
            return g | BinaryableFilter();
        else
            return g;
    }

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
        const Environment * const env;
        const ResolveCommandLineResolutionOptions & resolution_options;
        const bool from_set;
        const Resolvent resolvent;

        UseExistingVisitor(const Environment * const e, const ResolveCommandLineResolutionOptions & c, const bool f, const Resolvent & r) :
            env(e),
            resolution_options(c),
            from_set(f),
            resolvent(r)
        {
        }

        bool creating_and_no_appropriate_ids() const
        {
            bool (* can)(const std::shared_ptr<const PackageID> &)(0);
            switch (resolvent.destination_type())
            {
                case dt_install_to_slash:
                    return false;

                case dt_create_binary:
                    can = &can_make_binary_for;
                    break;

                case dt_install_to_chroot:
                    can = &can_chroot;
                    break;

                case last_dt:
                    break;
            }

            if (! can)
                throw InternalError(PALUDIS_HERE, "unhandled dt");

            auto origin_ids((*env)[selection::AllVersionsSorted(
                        generator::Package(resolvent.package()) |
                        make_slot_filter(resolvent) |
                        filter::SupportsAction<InstallAction>() |
                        filter::NotMasked()
                        )]);
            if (origin_ids->empty())
                return false;
            else
            {
                for (auto i(origin_ids->begin()), i_end(origin_ids->end()) ;
                        i != i_end ; ++i)
                    if ((*can)(*i))
                        return false;

                return true;
            }
        }

        std::pair<UseExisting, bool> visit(const DependencyReason &) const
        {
            return std::make_pair(use_existing_from_cmdline(resolution_options.a_keep, false),
                    creating_and_no_appropriate_ids());
        }

        std::pair<UseExisting, bool> visit(const TargetReason &) const
        {
            return std::make_pair(use_existing_from_cmdline(resolution_options.a_keep_targets, from_set),
                    creating_and_no_appropriate_ids());
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
            UseExistingVisitor v(env, resolution_options, true, resolvent);
            return r.reason_for_set()->accept_returning<std::pair<UseExisting, bool> >(v);
        }

        std::pair<UseExisting, bool> visit(const LikeOtherDestinationTypeReason & r) const
        {
            UseExistingVisitor v(env, resolution_options, true, resolvent);
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
            const std::shared_ptr<const Resolution> & resolution,
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

        UseExistingVisitor v(env, resolution_options, false, resolution->resolvent());
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
            const std::shared_ptr<const Generator> & binary_destinations,
            const Resolvent & q,
            const int n)
    {
        Context context("When working out whether '" + stringify(q) + "' has installed SCM packages:");

        const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                    make_destination_filtered_generator(env, resolution_options, binary_destinations,
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
            const std::shared_ptr<const Generator> & binary_destinations,
            const PackageDepSpecList & without,
            const Resolvent & resolvent)
    {
        const std::shared_ptr<Constraints> result(std::make_shared<Constraints>());

        int n(reinstall_scm_days(resolution_options));
        if ((-1 != n) && installed_is_scm_older_than(env, resolution_options, binary_destinations, resolvent, n)
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
            const std::shared_ptr<const Generator> & binary_destinations,
            const Resolvent & resolvent)
    {
        InitialConstraints::const_iterator i(initial_constraints.find(resolvent));
        if (i == initial_constraints.end())
            return make_initial_constraints_for(env, resolution_options, binary_destinations, without, resolvent);
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
                    (resolution_options.a_make.argument() == "chroot" ?
                     Filter(filter::InstalledAtNotSlash()) : Filter(filter::InstalledAtSlash())))]);

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

    Filter make_destination_filter_fn(const Resolvent & resolvent)
    {
        switch (resolvent.destination_type())
        {
            case dt_install_to_slash:
                return filter::InstalledAtSlash();

            case dt_install_to_chroot:
                return filter::InstalledAtNotSlash();

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
        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(
                const ChangesToMakeDecision & decision) const
        {
            return std::make_pair(decision.origin_id(), decision.if_changed_choices());
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(
                const BreakDecision & decision) const
        {
            return std::make_pair(decision.existing_id(), make_null_shared_ptr());
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(
                const ExistingNoChangeDecision & decision) const
        {
            return std::make_pair(decision.existing_id(), make_null_shared_ptr());
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(
                const NothingNoChangeDecision &) const
        {
            return std::make_pair(make_null_shared_ptr(), make_null_shared_ptr());
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(
                const RemoveDecision &) const
        {
            return std::make_pair(make_null_shared_ptr(), make_null_shared_ptr());
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(
                const UnableToMakeDecision &) const
        {
            return std::make_pair(make_null_shared_ptr(), make_null_shared_ptr());
        }
    };

    Tribool order_early_fn(
            const Environment * const env,
            const ResolveCommandLineResolutionOptions &,
            const PackageDepSpecList & early,
            const PackageDepSpecList & late,
            const std::shared_ptr<const Resolution> & r)
    {
        const std::shared_ptr<const PackageID> id(
                r->decision()->accept_returning<std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > >(
                    ChosenIDVisitor()).first);
        if (id)
        {
            if (match_any(env, early, id))
                return true;
            if (match_any(env, late, id))
                return false;
        }

        return indeterminate;
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
            auto c(r->previous_decision()->accept_returning<std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > >(
                        ChosenIDVisitor()));
            if (c.first)
                std::cout << *c.first;
            else
                std::cout << r->previous_decision()->accept_returning<std::string>(KindNameVisitor());

            if (c.second)
                std::cout << " (with changed choices)";

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
    PackageDepSpecList with, without, take, take_from, ignore, ignore_from,
                       favour, avoid, early, late, no_dependencies_from, no_blockers_from;

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

    std::shared_ptr<Generator> binary_destinations;
    for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
            r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
        if (! (*r)->installed_root_key())
        {
            if ((*r)->destination_interface())
            {
                if (binary_destinations)
                    binary_destinations = std::make_shared<generator::Union>(*binary_destinations,
                                generator::InRepository((*r)->name()));
                else
                    binary_destinations = std::make_shared<generator::InRepository>((*r)->name());
            }
        }

    if (! binary_destinations)
        binary_destinations = std::make_shared<generator::Nothing>();

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
                            env.get(), resolution_options, binary_destinations, without, *r))).first->second->add(
                    constraint);
        }
    }

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    using std::placeholders::_4;

    AllowChoiceChangesHelper allow_choice_changes_helper(env.get());;
    allow_choice_changes_helper.set_allow_choice_changes(! resolution_options.a_no_override_flags.specified());

    AllowedToRemoveHelper allowed_to_remove_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_uninstall.begin_args()),
            i_end(resolution_options.a_permit_uninstall.end_args()) ;
            i != i_end ; ++i)
        allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    AlwaysViaBinaryHelper always_via_binary_helper(env.get());
#ifdef ENABLE_PBINS
    for (args::StringSetArg::ConstIterator i(resolution_options.a_via_binary.begin_args()),
            i_end(resolution_options.a_via_binary.end_args()) ;
            i != i_end ; ++i)
        always_via_binary_helper.add_always_via_binary_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));
#endif

    CanUseHelper can_use_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_not_usable.begin_args()),
            i_end(resolution_options.a_not_usable.end_args()) ;
            i != i_end ; ++i)
        can_use_helper.add_cannot_use_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    ConfirmHelper confirm_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_downgrade.begin_args()),
            i_end(resolution_options.a_permit_downgrade.end_args()) ;
            i != i_end ; ++i)
        confirm_helper.add_permit_downgrade_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));
    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_old_version.begin_args()),
            i_end(resolution_options.a_permit_old_version.end_args()) ;
            i != i_end ; ++i)
        confirm_helper.add_permit_old_version_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));
    for (args::StringSetArg::ConstIterator i(resolution_options.a_uninstalls_may_break.begin_args()),
            i_end(resolution_options.a_uninstalls_may_break.end_args()) ;
            i != i_end ; ++i)
        if (*i == "system")
            confirm_helper.set_allowed_to_break_system(true);
        else
            confirm_helper.add_allowed_to_break_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    FindRepositoryForHelper find_repository_for_helper(env.get());

    GetConstraintsForDependentHelper get_constraints_for_dependent_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_less_restrictive_remove_blockers.begin_args()),
            i_end(resolution_options.a_less_restrictive_remove_blockers.end_args()) ;
            i != i_end ; ++i)
        get_constraints_for_dependent_helper.add_less_restrictive_remove_blockers_spec(
                parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    GetConstraintsForPurgeHelper get_constraints_for_purge_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_purge.begin_args()),
            i_end(resolution_options.a_purge.end_args()) ;
            i != i_end ; ++i)
        get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    GetConstraintsForViaBinaryHelper get_constraints_for_via_binary_helper(env.get());

    GetDestinationTypesForErrorHelper get_destination_types_for_error_helper(env.get());
    if (resolution_options.a_make.argument() == "binaries")
        get_destination_types_for_error_helper.set_target_destination_type(dt_create_binary);
    else if (resolution_options.a_make.argument() == "install")
        get_destination_types_for_error_helper.set_target_destination_type(dt_install_to_slash);
    else if (resolution_options.a_make.argument() == "chroot")
        get_destination_types_for_error_helper.set_target_destination_type(dt_install_to_chroot);
    else
        throw args::DoHelp("Don't understand argument '" + resolution_options.a_make.argument() + "' to '--"
                + resolution_options.a_make.long_name() + "'");

    RemoveIfDependentHelper remove_if_dependent_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_remove_if_dependent.begin_args()),
            i_end(resolution_options.a_remove_if_dependent.end_args()) ;
            i != i_end ; ++i)
        remove_if_dependent_helper.add_remove_if_dependent_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    ResolverFunctions resolver_functions(make_named_values<ResolverFunctions>(
                n::allow_choice_changes_fn() = std::cref(allow_choice_changes_helper),
                n::allowed_to_remove_fn() = std::cref(allowed_to_remove_helper),
                n::always_via_binary_fn() = std::cref(always_via_binary_helper),
                n::can_use_fn() = std::cref(can_use_helper),
                n::confirm_fn() = std::cref(confirm_helper),
                n::find_repository_for_fn() = std::cref(find_repository_for_helper),
                n::get_constraints_for_dependent_fn() = std::cref(get_constraints_for_dependent_helper),
                n::get_constraints_for_purge_fn() = std::cref(get_constraints_for_purge_helper),
                n::get_constraints_for_via_binary_fn() = std::cref(get_constraints_for_via_binary_helper),
                n::get_destination_types_for_error_fn() = std::cref(get_destination_types_for_error_helper),
                n::get_initial_constraints_for_fn() = std::bind(&initial_constraints_for_fn,
                    env.get(), std::cref(resolution_options), std::cref(without),
                    std::cref(initial_constraints), binary_destinations, _1),
                n::get_resolvents_for_fn() = std::bind(&get_resolvents_for_fn,
                    env.get(), std::cref(resolution_options), _1, _2, _3, DestinationTypes()),
                n::get_use_existing_nothing_fn() = std::bind(&use_existing_nothing_fn,
                    env.get(), std::cref(resolution_options), std::cref(without), std::cref(with), _1, _2, _3),
                n::interest_in_spec_fn() = std::bind(&interest_in_spec_fn,
                    env.get(), std::cref(resolution_options), std::cref(take), std::cref(take_from),
                    std::cref(ignore), std::cref(ignore_from), std::cref(no_blockers_from),
                    std::cref(no_dependencies_from), _1, _2),
                n::make_destination_filtered_generator_fn() = std::bind(&make_destination_filtered_generator_with_resolution,
                    env.get(), std::cref(resolution_options), binary_destinations, _1, _2),
                n::make_origin_filtered_generator_fn() = std::bind(&make_origin_filtered_generator,
                    env.get(), std::cref(resolution_options), _1, _2),
                n::make_unmaskable_filter_fn() = std::bind(&make_unmaskable_filter_fn,
                        env.get(), std::cref(resolution_options), _1),
                n::order_early_fn() = std::bind(&order_early_fn,
                    env.get(), std::cref(resolution_options), std::cref(early), std::cref(late), _1),
                n::prefer_or_avoid_fn() = std::bind(&prefer_or_avoid_fn,
                    env.get(), std::cref(resolution_options), std::cref(favour), std::cref(avoid), _1),
                n::remove_if_dependent_fn() = std::cref(remove_if_dependent_helper)
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
                                    env.get(), resolution_options, binary_destinations, without, e.resolvent()))).first->second->add(
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

