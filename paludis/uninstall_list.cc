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

#include <paludis/uninstall_list.hh>
#include <paludis/environment.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/dep_tag.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/dep_spec_flattener.hh>
#include <unordered_map>
#include <list>
#include <algorithm>
#include <set>

using namespace paludis;

#include <paludis/uninstall_list-se.cc>

typedef std::unordered_map<
    std::shared_ptr<const PackageID>,
    std::shared_ptr<const DepListEntryTags>,
    Hash<std::shared_ptr<const PackageID> > > DepCollectorCache;

namespace paludis
{
    template<>
    struct Imp<UninstallList>
    {
        const Environment * const env;
        UninstallListOptions options;
        std::list<UninstallListEntry> uninstall_list;

        mutable Mutex dep_collector_cache_mutex;
        mutable DepCollectorCache dep_collector_cache;

        Imp(const Environment * const e, const UninstallListOptions & o) :
            env(e),
            options(o)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<UninstallList::UninstallListTag>
    {
        typedef std::list<UninstallListEntry>::const_iterator UnderlyingIterator;
    };
}

namespace
{
    struct MatchUninstallListEntry
    {
        const std::shared_ptr<const PackageID> e;

        MatchUninstallListEntry(const std::shared_ptr<const PackageID> & ee) :
            e(ee)
        {
        }

        bool operator() (const UninstallListEntry & f) const
        {
            return *f.package_id() == *e;
        }
    };
}

UninstallList::UninstallList(const Environment * const env, const UninstallListOptions & o) :
    Pimp<UninstallList>(env, o),
    options(_imp->options)
{
}

UninstallList::~UninstallList()
{
}

void
UninstallList::add(const std::shared_ptr<const PackageID> & e, const std::shared_ptr<DepTag> & t)
{
    real_add(e, t, false);
}

void
UninstallList::real_add(const std::shared_ptr<const PackageID> & e, const std::shared_ptr<DepTag> & t,
        const bool error)
{
    std::list<UninstallListEntry>::iterator i;
    if (_imp->uninstall_list.end() != ((i = std::find_if(_imp->uninstall_list.begin(),
                        _imp->uninstall_list.end(), MatchUninstallListEntry(e)))))
    {
        if (t)
            i->tags()->insert(t);

        return;
    }

    Context context("When adding '" + stringify(*e) + "' to the uninstall list:");

    if ((! error) || (! e->virtual_for_key()))
        add_package(e, t, error ? ulk_requires : (e->virtual_for_key() ? ulk_virtual : ulk_package));

    if (! error)
    {
        /* don't recurse errors, it gets horrid */
        if (_imp->options.with_dependencies_included())
            add_dependencies(*e, false);
        else if (_imp->options.with_dependencies_as_errors())
            add_dependencies(*e, true);
    }

    move_package_to_end(e);

    if (_imp->options.with_unused_dependencies())
        add_unused_dependencies();
}

void
UninstallList::add_errors_for_system()
{
    Context context("When finding system packages:");

    std::shared_ptr<const SetSpecTree> system(_imp->env->set(SetName("system")));
    if (! system)
        return;

    std::shared_ptr<Set<std::shared_ptr<DepTag> > > tags(std::make_shared<Set<std::shared_ptr<DepTag> >>());
    tags->insert(std::make_shared<GeneralSetDepTag>(SetName("system"), ""));

    for (std::list<UninstallListEntry>::iterator l(_imp->uninstall_list.begin()), l_end(_imp->uninstall_list.end()) ;
            l != l_end ; ++l)
    {
        if (l->kind() == ulk_requires)
            continue;

        bool needed(false);
        if (match_package_in_set(*_imp->env, *system, *l->package_id(), { }))
            needed = true;

        if ((! needed) && l->package_id()->provide_key())
        {
            DepSpecFlattener<ProvideSpecTree, PackageDepSpec> f(_imp->env);
            l->package_id()->provide_key()->value()->top()->accept(f);
            for (DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIterator v(f.begin()), v_end(f.end()) ;
                    v != v_end && ! needed ; ++v)
            {
                const std::shared_ptr<const PackageIDSequence> virtuals((*_imp->env)[selection::AllVersionsUnsorted(
                            generator::Matches(**v, { }))]);
                for (PackageIDSequence::ConstIterator i(virtuals->begin()), i_end(virtuals->end()) ;
                        i != i_end && ! needed ; ++i)
                    if (match_package_in_set(*_imp->env, *system, **i, { }))
                        needed = true;
            }
        }

        if (needed)
            _imp->uninstall_list.insert(l, make_named_values<UninstallListEntry>(
                    n::kind() = ulk_required_by,
                    n::package_id() = l->package_id(),
                    n::tags() = tags
                    ));
    }
}

void
UninstallList::add_unused()
{
    Context context("When finding unused packages:");

    std::shared_ptr<const PackageIDSet> world(collect_world()),
        everything(collect_all_installed()), used(collect_used());

    std::shared_ptr<PackageIDSet>
        world_plus_deps(std::make_shared<PackageIDSet>()),
        unused(std::make_shared<PackageIDSet>());

    std::copy(world->begin(), world->end(), world_plus_deps->inserter());
    std::copy(used->begin(), used->end(), world_plus_deps->inserter());

    std::size_t old_size(0);
    while (old_size != world_plus_deps->size())
    {
        old_size = world_plus_deps->size();
        std::shared_ptr<const PackageIDSet> new_world_deps(collect_depped_upon(world_plus_deps));
        std::copy(new_world_deps->begin(), new_world_deps->end(), world_plus_deps->inserter());
    }

    std::set_difference(everything->begin(), everything->end(),
            world_plus_deps->begin(), world_plus_deps->end(), unused->inserter(),
            PackageIDSetComparator());

    for (PackageIDSet::ConstIterator i(unused->begin()), i_end(unused->end()) ; i != i_end ; ++i)
        add_package(*i, std::shared_ptr<DepTag>(), (*i)->virtual_for_key() ? ulk_virtual : ulk_package);
}

UninstallList::ConstIterator
UninstallList::begin() const
{
    return ConstIterator(_imp->uninstall_list.begin());
}

UninstallList::ConstIterator
UninstallList::end() const
{
    return ConstIterator(_imp->uninstall_list.end());
}

void
UninstallList::add_package(const std::shared_ptr<const PackageID> & e, const std::shared_ptr<DepTag> & t,
        const UninstallListEntryKind k)
{
    Context context("When adding package '" + stringify(*e) + "' to the uninstall list:");

    std::list<UninstallListEntry>::iterator i(_imp->uninstall_list.insert(
                _imp->uninstall_list.end(), make_named_values<UninstallListEntry>(
                    n::kind() = k,
                    n::package_id() = e,
                    n::tags() = std::make_shared<Set<std::shared_ptr<DepTag> >>()
                    )));

    if (t)
        i->tags()->insert(t);
}

void
UninstallList::move_package_to_end(const std::shared_ptr<const PackageID> & e)
{
    Context context("When removing package '" + stringify(*e) + "' from the uninstall list:");

    std::list<UninstallListEntry>::iterator i(std::find_if(_imp->uninstall_list.begin(),
                _imp->uninstall_list.end(), MatchUninstallListEntry(e)));
    if (_imp->uninstall_list.end() != i)
        _imp->uninstall_list.splice(_imp->uninstall_list.end(), _imp->uninstall_list, i);
}

std::shared_ptr<const PackageIDSet>
UninstallList::collect_all_installed() const
{
    Context context("When collecting all installed packages:");

    std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());
    for (PackageDatabase::RepositoryConstIterator i(_imp->env->package_database()->begin_repositories()),
            i_end(_imp->env->package_database()->end_repositories()) ; i != i_end ; ++i)
    {
        if (! (*i)->installed_root_key())
            continue;

        std::shared_ptr<const CategoryNamePartSet> cats((*i)->category_names());
        for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                c != c_end ; ++c)
        {
            std::shared_ptr<const QualifiedPackageNameSet> pkgs((*i)->package_names(*c));
            for (QualifiedPackageNameSet::ConstIterator p(pkgs->begin()), p_end(pkgs->end()) ;
                    p != p_end ; ++p)
            {
                std::shared_ptr<const PackageIDSequence> ids((*i)->package_ids(*p));
                std::copy(ids->begin(), ids->end(), result->inserter());
            }
        }
    }

    return result;
}

namespace
{
    struct DepCollector
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> pkg;
        std::shared_ptr<DepListEntryTags> matches;
        std::set<SetName> recursing_sets;

        DepCollector(const Environment * const ee, const std::shared_ptr<const PackageID> & e) :
            env(ee),
            pkg(e),
            matches(std::make_shared<DepListEntryTags>())
        {
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()),
                    accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            bool best_only(false);
            if (node.spec()->slot_requirement_ptr())
                best_only = simple_visitor_cast<const SlotAnyUnlockedRequirement>(*node.spec()->slot_requirement_ptr());

            std::shared_ptr<const PackageIDSequence> m(
                    best_only ?
                    (*env)[selection::BestVersionOnly(generator::Matches(*node.spec(), { }) | filter::InstalledAtRoot(env->preferred_root_key()->value()))] :
                    (*env)[selection::AllVersionsSorted(generator::Matches(*node.spec(), { }) | filter::InstalledAtRoot(env->preferred_root_key()->value()))]);
            for (PackageIDSequence::ConstIterator it = m->begin(), it_end = m->end();
                 it_end != it; ++it)
                matches->insert(make_named_values<DepTagEntry>(
                            n::generation() = 0,
                            n::tag() = std::shared_ptr<const DepTag>(std::make_shared<DependencyDepTag>(*it, *node.spec()))
                            ));
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()),
                    accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (node.spec()->condition_met())
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()),
                        accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
        {
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
        {
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            Context context("When expanding named set '" + stringify(*node.spec()) + "':");

            std::shared_ptr<const SetSpecTree> set(env->set(node.spec()->name()));

            if (! set)
            {
                Log::get_instance()->message("uninstall_list.unknown_set", ll_warning, lc_context) << "Unknown set '" << node.spec()->name() << "'";
                return;
            }

            if (! recursing_sets.insert(node.spec()->name()).second)
            {
                Log::get_instance()->message("uninstall_list.recursive_set", ll_warning, lc_context)
                    << "Recursively defined set '" << node.spec()->name() << "'";
                return;
            }

            set->top()->accept(*this);

            recursing_sets.erase(node.spec()->name());
        }
    };
}

std::shared_ptr<const PackageIDSet>
UninstallList::collect_depped_upon(std::shared_ptr<const PackageIDSet> targets) const
{
    Context context("When collecting depended upon packages:");

    std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

    Lock l(_imp->dep_collector_cache_mutex);
    for (PackageIDSet::ConstIterator i(targets->begin()), i_end(targets->end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When collecting depended upon packages for '" + stringify(**i) + "':");

        DepCollectorCache::const_iterator cache(_imp->dep_collector_cache.find(*i));

        if (cache == _imp->dep_collector_cache.end())
        {
            DepCollector c(_imp->env, *i);
            if ((*i)->build_dependencies_key())
                (*i)->build_dependencies_key()->value()->top()->accept(c);
            if ((*i)->run_dependencies_key())
                (*i)->run_dependencies_key()->value()->top()->accept(c);
            if ((*i)->post_dependencies_key())
                (*i)->post_dependencies_key()->value()->top()->accept(c);
            if ((*i)->suggested_dependencies_key())
                (*i)->suggested_dependencies_key()->value()->top()->accept(c);

            cache = _imp->dep_collector_cache.insert(std::make_pair(*i,
                        std::shared_ptr<const DepListEntryTags>(c.matches))).first;
        }

        for (DepListEntryTags::ConstIterator it(cache->second->begin()), it_end(cache->second->end());
             it_end != it; ++it)
            result->insert(std::static_pointer_cast<const DependencyDepTag>(it->tag())->package_id());
    }

    return result;
}

void
UninstallList::add_unused_dependencies()
{
    Context context("When adding unused dependencies:");

    bool added(true);
    std::shared_ptr<const PackageIDSet> everything(collect_all_installed());
    while (added)
    {
        added = false;

        /* find packages that're depped upon by anything in our uninstall list, excluding error
         * packages */
        std::shared_ptr<PackageIDSet> uninstall_list_targets(std::make_shared<PackageIDSet>());
        for (std::list<UninstallListEntry>::const_iterator i(_imp->uninstall_list.begin()),
                i_end(_imp->uninstall_list.end()) ; i != i_end ; ++i)
            if (i->kind() == ulk_package || i->kind() == ulk_virtual)
                uninstall_list_targets->insert(i->package_id());

        std::shared_ptr<const PackageIDSet> depped_upon_list(collect_depped_upon(uninstall_list_targets));

        /* find packages that're depped upon by anything not in our uninstall list */
        std::shared_ptr<PackageIDSet> everything_except_uninstall_list_targets(std::make_shared<PackageIDSet>());
        std::set_difference(everything->begin(), everything->end(),
                uninstall_list_targets->begin(), uninstall_list_targets->end(),
                everything_except_uninstall_list_targets->inserter(),
                PackageIDSetComparator());

        Log::get_instance()->message("uninstall_list.everything_except_uninstall_list_targets", ll_debug, lc_context)
            << "everything_except_uninstall_list_targets is '"
            << join(indirect_iterator(everything_except_uninstall_list_targets->begin()),
                    indirect_iterator(everything_except_uninstall_list_targets->end()), " ") << "'";

        std::shared_ptr<const PackageIDSet> depped_upon_not_list(
                collect_depped_upon(everything_except_uninstall_list_targets));

        /* find unused dependencies */
        std::shared_ptr<PackageIDSet> unused_dependencies(std::make_shared<PackageIDSet>());
        std::set_difference(depped_upon_list->begin(), depped_upon_list->end(),
                depped_upon_not_list->begin(), depped_upon_not_list->end(), unused_dependencies->inserter(),
                PackageIDSetComparator());

        /* if any of them aren't already on the list, and aren't in world, add them and recurse */
        std::shared_ptr<const SetSpecTree> world(_imp->env->set(SetName("world")));
        for (PackageIDSet::ConstIterator i(unused_dependencies->begin()),
                i_end(unused_dependencies->end()) ; i != i_end ; ++i)
        {
            if (match_package_in_set(*_imp->env, *world, **i, { }))
                continue;

            if (_imp->uninstall_list.end() != std::find_if(_imp->uninstall_list.begin(),
                        _imp->uninstall_list.end(), MatchUninstallListEntry(*i)))
                continue;

            add_package(*i, std::shared_ptr<DepTag>(), (*i)->virtual_for_key() ? ulk_virtual : ulk_package);
            added = true;
        }
    }
}

void
UninstallList::add_dependencies(const PackageID & e, const bool error)
{
    Context context("When adding things that depend upon '" + stringify(e) + "':");

    std::shared_ptr<const PackageIDSet> everything(collect_all_installed());

    Lock l(_imp->dep_collector_cache_mutex);

    for (PackageIDSet::ConstIterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
    {
        Context local_context("When seeing whether '" + stringify(**i) + "' has a dep:");

        DepCollectorCache::const_iterator cache(_imp->dep_collector_cache.find(*i));

        if (cache == _imp->dep_collector_cache.end())
        {
            DepCollector c(_imp->env, *i);
            if ((*i)->build_dependencies_key())
                (*i)->build_dependencies_key()->value()->top()->accept(c);
            if ((*i)->run_dependencies_key())
                (*i)->run_dependencies_key()->value()->top()->accept(c);
            if ((*i)->post_dependencies_key())
                (*i)->post_dependencies_key()->value()->top()->accept(c);
            if ((*i)->suggested_dependencies_key())
                (*i)->suggested_dependencies_key()->value()->top()->accept(c);
            cache = _imp->dep_collector_cache.insert(std::make_pair(*i,
                        std::shared_ptr<const DepListEntryTags>(c.matches))).first;
        }

        bool logged(false);
        for (DepListEntryTags::ConstIterator it(cache->second->begin()), it_end(cache->second->end());
             it_end != it; ++it)
        {
            std::shared_ptr<const DependencyDepTag> tag(std::static_pointer_cast<const DependencyDepTag>(it->tag()));
            if (*tag->package_id() == e)
            {
                if (! logged)
                {
                    Log::get_instance()->message("uninstall_list.adding", ll_debug, lc_context) << "Adding '" << **i <<
                            "' because it depends upon '" << e << "'";
                    logged = true;
                }
                real_add(*i, std::shared_ptr<DependencyDepTag>(
                        std::make_shared<DependencyDepTag>(tag->package_id(), *tag->dependency())), error);
            }
        }

    }
}

std::shared_ptr<const PackageIDSet>
UninstallList::collect_world() const
{
    Context local_context("When collecting world packages:");

    std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());
    std::shared_ptr<const PackageIDSet> everything(collect_all_installed());

    std::shared_ptr<const SetSpecTree> world(_imp->env->set(SetName("world")));
    for (PackageIDSet::ConstIterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
    {
        if (match_package_in_set(*_imp->env, *world, **i, { }))
            result->insert(*i);
    }

    return result;
}

std::shared_ptr<const PackageIDSet>
UninstallList::collect_used() const
{
    Context local_context("When collecting used packages:");

    std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());
    std::shared_ptr<const PackageIDSet> everything(collect_all_installed());

    for (PackageIDSet::ConstIterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
    {
        if ((*i)->behaviours_key() && (*i)->behaviours_key()->value()->end() !=
                (*i)->behaviours_key()->value()->find("used"))
            result->insert(*i);
    }

    return result;
}

namespace
{
    struct IsError
    {
        bool operator() (const UninstallListEntry & e) const
        {
            switch (e.kind())
            {
                case ulk_virtual:
                case ulk_package:
                    return false;

                case ulk_requires:
                case ulk_required_by:
                    return true;

                case last_ulk:
                    ;
            }

            throw InternalError(PALUDIS_HERE, "Bad e.kind");
        }
    };
}

bool
UninstallList::has_errors() const
{
    return end() != std::find_if(begin(), end(), IsError());
}

template class Set<std::shared_ptr<DepTag> >;
template class WrappedForwardIterator<Set<std::shared_ptr<DepTag> >::ConstIteratorTag, const std::shared_ptr<DepTag> >;
template class WrappedOutputIterator<Set<std::shared_ptr<DepTag> >::InserterTag, std::shared_ptr<DepTag> >;
template class WrappedForwardIterator<UninstallList::UninstallListTag, const UninstallListEntry>;

