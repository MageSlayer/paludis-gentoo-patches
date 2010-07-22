/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/uninstall_task.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <paludis/uninstall_list.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/tasks_exceptions.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/package_database.hh>
#include <paludis/hook.hh>
#include <paludis/dep_tag.hh>
#include <paludis/repository.hh>
#include <paludis/output_manager_from_environment.hh>
#include <paludis/output_manager.hh>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <functional>

using namespace paludis;

AmbiguousUnmergeTargetError::AmbiguousUnmergeTargetError(const std::string & t,
        const std::shared_ptr<const PackageIDSequence> m) throw () :
    Exception("Ambiguous unmerge target '" + t + "'"),
    _t(t),
    _p(m)
{
}

AmbiguousUnmergeTargetError::~AmbiguousUnmergeTargetError() throw ()
{
}

AmbiguousUnmergeTargetError::ConstIterator
AmbiguousUnmergeTargetError::begin() const
{
    return ConstIterator(_p->begin());
}

AmbiguousUnmergeTargetError::ConstIterator
AmbiguousUnmergeTargetError::end() const
{
    return ConstIterator(_p->end());
}

std::string
AmbiguousUnmergeTargetError::target() const
{
    return _t;
}

namespace paludis
{
    template<>
    struct Implementation<UninstallTask>
    {
        Environment * const env;

        std::list<std::string> raw_targets;
        std::list<std::shared_ptr<const PackageDepSpec> > targets;

        bool pretend;
        bool preserve_world;
        bool all_versions;
        bool with_unused_dependencies;
        bool with_dependencies;
        bool unused;
        bool check_safety;

        bool had_set_targets;
        bool had_package_targets;

        Implementation<UninstallTask>(Environment * const e) :
            env(e),
            pretend(false),
            preserve_world(false),
            all_versions(false),
            with_unused_dependencies(false),
            with_dependencies(false),
            unused(false),
            check_safety(false),
            had_set_targets(false),
            had_package_targets(false)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<AmbiguousUnmergeTargetError::ConstIteratorTag>
    {
        typedef PackageIDSequence::ConstIterator UnderlyingIterator;
    };
}

UninstallTask::UninstallTask(Environment * const e) :
    PrivateImplementationPattern<UninstallTask>(new Implementation<UninstallTask>(e))
{
}

UninstallTask::~UninstallTask()
{
}

void
UninstallTask::set_pretend(const bool v)
{
    _imp->pretend = v;
}

void
UninstallTask::set_preserve_world(const bool v)
{
    _imp->preserve_world = v;
}

void
UninstallTask::set_check_safety(const bool v)
{
    _imp->check_safety = v;
}

void
UninstallTask::add_target(const std::string & target)
{
    Context context("When adding uninstall target '" + target + "':");

    try
    {
        std::shared_ptr<PackageDepSpec> pds(new PackageDepSpec(parse_user_package_dep_spec(
                        target, _imp->env, UserPackageDepSpecOptions() + updso_throw_if_set + updso_allow_wildcards,
                        filter::SupportsAction<UninstallAction>())));

        if (_imp->had_set_targets)
            throw HadBothPackageAndSetTargets();
        _imp->had_package_targets = true;

        if (pds->package_ptr())
        {
            /* don't need to dewildcard */
            pds->set_tag(std::shared_ptr<const DepTag>(new TargetDepTag));
            _imp->targets.push_back(pds);
        }
        else
        {
            /* blech. wildcards. */
            std::shared_ptr<const PackageIDSequence> names((*_imp->env)[selection::BestVersionOnly(
                        generator::Matches(*pds, MatchPackageOptions()) | filter::SupportsAction<UninstallAction>())]);
            if (names->empty())
            {
                /* no match. we'll get an error from this later anyway. */
                pds->set_tag(std::shared_ptr<const DepTag>(new TargetDepTag));
                _imp->targets.push_back(pds);
            }
            else
            {
                for (PackageIDSequence::ConstIterator i(names->begin()), i_end(names->end()) ;
                        i != i_end ; ++i)
                {
                    PartiallyMadePackageDepSpec p(*pds);
                    p.package((*i)->name());
                    std::shared_ptr<PackageDepSpec> pdsn(new PackageDepSpec(p));
                    pdsn->set_tag(std::shared_ptr<const DepTag>(new TargetDepTag));
                    _imp->targets.push_back(pdsn);
                }
            }
        }
    }
    catch (const GotASetNotAPackageDepSpec &)
    {
        if (_imp->had_set_targets)
            throw MultipleSetTargetsSpecified();
        if (_imp->had_package_targets)
            throw HadBothPackageAndSetTargets();
        _imp->had_set_targets = true;

        std::shared_ptr<const SetSpecTree> spec(_imp->env->set(SetName(target)));
        DepSpecFlattener<SetSpecTree, PackageDepSpec> f(_imp->env);
        spec->root()->accept(f);
        std::copy(f.begin(), f.end(), std::back_inserter(_imp->targets));
    }

    _imp->raw_targets.push_back(target);

    if (_imp->unused)
        throw InternalError(PALUDIS_HERE, "Trying to mix unused and normal targets?");
}

void
UninstallTask::add_unused()
{
    Context context("When adding unused packages:");
    _imp->unused = true;

    if (! _imp->raw_targets.empty())
        throw InternalError(PALUDIS_HERE, "Trying to mix unused and normal targets?");
}

namespace
{
    bool ignore_nothing(const FSEntry &)
    {
        return false;
    }
}

void
UninstallTask::execute()
{
    using namespace std::placeholders;

    Context context("When executing uninstall task:");

    on_build_unmergelist_pre();

    UninstallList list(_imp->env, make_named_values<UninstallListOptions>(
            n::with_dependencies_as_errors() = _imp->check_safety,
            n::with_dependencies_included() = _imp->with_dependencies,
            n::with_unused_dependencies() = _imp->with_unused_dependencies
            ));

    if (_imp->unused)
        list.add_unused();
    else
    {
        for (std::list<std::shared_ptr<const PackageDepSpec> >::const_iterator t(_imp->targets.begin()),
                t_end(_imp->targets.end()) ; t != t_end ; ++t)
        {
            Context local_context("When looking for target '" + stringify(**t) + "':");

            std::shared_ptr<const PackageIDSequence> r((*_imp->env)[selection::AllVersionsSorted(
                        generator::Matches(**t, MatchPackageOptions()) |
                        filter::SupportsAction<UninstallAction>())]);
            if (r->empty())
            {
                if (! _imp->had_set_targets)
                    throw NoSuchPackageError(stringify(**t));
            }
            else if (next(r->begin()) != r->end())
            {
                if (_imp->all_versions)
                {
                    /* all_versions, not all_packages. */
                    for (PackageIDSequence::ConstIterator i_start(r->begin()), i(r->begin()),
                            i_end(r->end()) ; i != i_end ; ++i)
                        if ((*i)->name() != (*i_start)->name())
                            throw AmbiguousUnmergeTargetError(stringify(**t), r);

                    for (PackageIDSequence::ConstIterator i(r->begin()), i_end(r->end()) ;
                            i != i_end ; ++i)
                        list.add(*i);
                }
                else
                    throw AmbiguousUnmergeTargetError(stringify(**t), r);
            }
            else
                list.add(*r->begin());
        }

        if (_imp->check_safety)
            list.add_errors_for_system();
    }

    on_build_unmergelist_post();

    on_display_unmerge_list_pre();

    for (UninstallList::ConstIterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
        on_display_unmerge_list_entry(*i);

    on_display_unmerge_list_post();

    if (_imp->pretend)
        return;

    if (list.has_errors())
    {
        on_not_continuing_due_to_errors();
        return;
    }

    if (_imp->preserve_world)
        on_preserve_world();
    else
    {
        on_update_world_pre();

        std::shared_ptr<SetSpecTree> all(new SetSpecTree(make_shared_ptr(new AllDepSpec)));

        std::map<QualifiedPackageName, std::set<VersionSpec> > being_removed;
        for (UninstallList::ConstIterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
            if (i->kind() != ulk_virtual)
                being_removed[i->package_id()->name()].insert(i->package_id()->version());

        for (std::map<QualifiedPackageName, std::set<VersionSpec> >::const_iterator
                i(being_removed.begin()), i_end(being_removed.end()) ; i != i_end ; ++i)
        {
            bool remove(true);
            std::shared_ptr<const PackageIDSequence> installed((*_imp->env)[selection::AllVersionsUnsorted(
                        generator::Matches(make_package_dep_spec(PartiallyMadePackageDepSpecOptions()).package(i->first), MatchPackageOptions()) |
                        filter::InstalledAtRoot(_imp->env->root())
                        )]);
            for (PackageIDSequence::ConstIterator r(installed->begin()), r_end(installed->end()) ;
                    r != r_end && remove ; ++r)
                if (i->second.end() == i->second.find((*r)->version()))
                    remove = false;

            if (remove)
                all->root()->append(make_shared_ptr(new PackageDepSpec(make_package_dep_spec(
                                    PartiallyMadePackageDepSpecOptions()).package(i->first))));
        }

        world_remove_packages(all);

        if (_imp->had_set_targets)
            for (std::list<std::string>::const_iterator t(_imp->raw_targets.begin()),
                    t_end(_imp->raw_targets.end()) ; t != t_end ; ++t)
                world_remove_set(SetName(*t));

        on_update_world_post();
    }

    if (0 !=
        _imp->env->perform_hook(Hook("uninstall_all_pre")("TARGETS", join(_imp->raw_targets.begin(),
                         _imp->raw_targets.end(), " "))).max_exit_status())
        throw ActionAbortedError("Uninstall aborted by hook");
    on_uninstall_all_pre();

    int x(0), y(0);
    for (UninstallList::ConstIterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
        if (i->kind() != ulk_virtual)
            ++y;

    for (UninstallList::ConstIterator i(list.begin()), i_end(list.end()) ; i != i_end ; ++i)
    {
        if (i->kind() == ulk_virtual)
            continue;
        ++x;

        std::string cpvr(stringify(*i->package_id()));

        if (0 !=
            _imp->env->perform_hook(Hook("uninstall_pre")("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status())
            throw ActionAbortedError("Uninstall of '" + cpvr + "' aborted by hook");
        on_uninstall_pre(*i);

        try
        {
            OutputManagerFromEnvironment output_manager_holder(_imp->env, i->package_id(), oe_exclusive,
                    ClientOutputFeatures());
            UninstallAction uninstall_action(
                    make_named_values<UninstallActionOptions>(
                        n::config_protect() = "",
                        n::if_for_install_id() = make_null_shared_ptr(),
                        n::ignore_for_unmerge() = &ignore_nothing,
                        n::is_overwrite() = false,
                        n::make_output_manager() = std::ref(output_manager_holder)
                        ));
            i->package_id()->perform_action(uninstall_action);

            if (output_manager_holder.output_manager_if_constructed())
                output_manager_holder.output_manager_if_constructed()->succeeded();
        }
        catch (const ActionFailedError & e)
        {
            HookResult PALUDIS_ATTRIBUTE((unused)) dummy(_imp->env->perform_hook(Hook("uninstall_fail")("TARGET", cpvr)("MESSAGE", e.message())));
            throw;
        }

        on_uninstall_post(*i);
        if (0 !=
            _imp->env->perform_hook(Hook("uninstall_post")("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status())
            throw ActionAbortedError("Uninstall of '" + cpvr + "' aborted by hook");
    }

    on_uninstall_all_post();
    if (0 !=
        _imp->env->perform_hook(Hook("uninstall_all_post")("TARGETS", join(_imp->raw_targets.begin(),
                         _imp->raw_targets.end(), " "))).max_exit_status())
        throw ActionAbortedError("Uninstall aborted by hook");
}

void
UninstallTask::set_with_unused_dependencies(const bool value)
{
    _imp->with_unused_dependencies = value;
}

void
UninstallTask::set_with_dependencies(const bool value)
{
    _imp->with_dependencies = value;
}

void
UninstallTask::set_all_versions(const bool value)
{
    _imp->all_versions = value;
}

void
UninstallTask::world_remove_set(const SetName & s)
{
    _imp->env->remove_from_world(s);
    on_update_world(s);
}

namespace
{
    struct WorldTargetFinder
    {
        Environment * const env;
        UninstallTask * const task;

        WorldTargetFinder(Environment * const e, UninstallTask * const t) :
            env(e),
            task(t)
        {
        }

        void visit(const SetSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const SetSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            if (! (node.spec()->slot_requirement_ptr() || (node.spec()->version_requirements_ptr() && ! node.spec()->version_requirements_ptr()->empty())))
            {
                if (node.spec()->package_ptr())
                    env->remove_from_world(*node.spec()->package_ptr());

                task->on_update_world(*node.spec());
            }
        }

        void visit(const SetSpecTree::NodeType<NamedSetDepSpec>::Type &)
        {
        }
    };
}

void
UninstallTask::world_remove_packages(const std::shared_ptr<const SetSpecTree> & a)
{
    WorldTargetFinder w(_imp->env, this);
    a->root()->accept(w);
}

template class WrappedForwardIterator<AmbiguousUnmergeTargetError::ConstIteratorTag, const std::shared_ptr<const PackageID> >;

