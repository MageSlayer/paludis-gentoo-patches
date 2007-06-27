/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "uninstall_list.hh"

using namespace paludis;

#include <paludis/dep_list/uninstall_list-sr.cc>
#include <paludis/dep_list/condition_tracker.hh>
#include <paludis/environment.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/query.hh>
#include <paludis/metadata_key.hh>
#include <paludis/dep_tag.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <list>
#include <algorithm>

typedef MakeHashedMap<tr1::shared_ptr<const PackageID>, tr1::shared_ptr<const DepListEntryTags> >::Type DepCollectorCache;

namespace paludis
{
    template<>
    struct Implementation<UninstallList>
    {
        const Environment * const env;
        UninstallListOptions options;
        std::list<UninstallListEntry> uninstall_list;

        mutable DepCollectorCache dep_collector_cache;

        Implementation(const Environment * const e, const UninstallListOptions & o) :
            env(e),
            options(o)
        {
        }
    };
}

namespace
{
    struct MatchUninstallListEntry
    {
        const tr1::shared_ptr<const PackageID> e;

        MatchUninstallListEntry(const tr1::shared_ptr<const PackageID> & ee) :
            e(ee)
        {
        }

        bool operator() (const UninstallListEntry & f) const
        {
            return *f.package_id == *e;
        }
    };
}

UninstallList::UninstallList(const Environment * const env, const UninstallListOptions & o) :
    PrivateImplementationPattern<UninstallList>(new Implementation<UninstallList>(env, o)),
    options(_imp->options)
{
}

UninstallList::~UninstallList()
{
}

void
UninstallList::add(const tr1::shared_ptr<const PackageID> & e, tr1::shared_ptr<DepTag> t)
{
    std::list<UninstallListEntry>::iterator i;
    if (_imp->uninstall_list.end() != ((i = std::find_if(_imp->uninstall_list.begin(),
                        _imp->uninstall_list.end(), MatchUninstallListEntry(e)))))
    {
        if (t)
            i->tags->insert(t);

        return;
    }

    Context context("When adding '" + stringify(*e) + "' to the uninstall list:");

    add_package(e, t);

    if (_imp->options.with_dependencies)
        add_dependencies(*e);

    move_package_to_end(e);

    if (_imp->options.with_unused_dependencies)
        add_unused_dependencies();
}

void
UninstallList::add_unused()
{
    Context context("When finding unused packages:");

    tr1::shared_ptr<const PackageIDSet> world(collect_world()),
        everything(collect_all_installed());

    tr1::shared_ptr<PackageIDSet>
        world_plus_deps(new PackageIDSet::Concrete),
        unused(new PackageIDSet::Concrete);

    world_plus_deps->insert(world->begin(), world->end());

    std::size_t old_size(0);
    while (old_size != world_plus_deps->size())
    {
        old_size = world_plus_deps->size();
        tr1::shared_ptr<const PackageIDSet> new_world_deps(collect_depped_upon(world_plus_deps));
        world_plus_deps->insert(new_world_deps->begin(), new_world_deps->end());
    }

    std::set_difference(everything->begin(), everything->end(),
            world_plus_deps->begin(), world_plus_deps->end(), unused->inserter(),
            PackageIDSetComparator());

    for (PackageIDSet::Iterator i(unused->begin()), i_end(unused->end()) ; i != i_end ; ++i)
        add_package(*i, tr1::shared_ptr<DepTag>());
}

UninstallList::Iterator
UninstallList::begin() const
{
    return Iterator(_imp->uninstall_list.begin());
}

UninstallList::Iterator
UninstallList::end() const
{
    return Iterator(_imp->uninstall_list.end());
}

UninstallListOptions::UninstallListOptions() :
    with_unused_dependencies(false),
    with_dependencies(false)
{
}

void
UninstallList::add_package(const tr1::shared_ptr<const PackageID> & e, tr1::shared_ptr<DepTag> t)
{
    Context context("When adding package '" + stringify(*e) + "' to the uninstall list:");

    std::list<UninstallListEntry>::iterator i(_imp->uninstall_list.insert(
                _imp->uninstall_list.end(), UninstallListEntry(
                    e, e->virtual_for_key(), tr1::shared_ptr<SortedCollection<tr1::shared_ptr<DepTag> > >(
                        new SortedCollection<tr1::shared_ptr<DepTag> >::Concrete))));
    if (t)
        i->tags->insert(t);
}

void
UninstallList::move_package_to_end(const tr1::shared_ptr<const PackageID> & e)
{
    Context context("When removing package '" + stringify(*e) + "' from the uninstall list:");

    std::list<UninstallListEntry>::iterator i(std::find_if(_imp->uninstall_list.begin(),
                _imp->uninstall_list.end(), MatchUninstallListEntry(e)));
    if (_imp->uninstall_list.end() != i)
        _imp->uninstall_list.splice(_imp->uninstall_list.end(), _imp->uninstall_list, i);
}

tr1::shared_ptr<const PackageIDSet>
UninstallList::collect_all_installed() const
{
    Context context("When collecting all installed packages:");

    tr1::shared_ptr<PackageIDSet> result(new PackageIDSet::Concrete);
    for (PackageDatabase::RepositoryIterator i(_imp->env->package_database()->begin_repositories()),
            i_end(_imp->env->package_database()->end_repositories()) ; i != i_end ; ++i)
    {
        if (! (*i)->installed_interface)
            continue;

        tr1::shared_ptr<const CategoryNamePartCollection> cats((*i)->category_names());
        for (CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end()) ;
                c != c_end ; ++c)
        {
            tr1::shared_ptr<const QualifiedPackageNameCollection> pkgs((*i)->package_names(*c));
            for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                    p != p_end ; ++p)
            {
                tr1::shared_ptr<const PackageIDSequence> ids((*i)->package_ids(*p));
                std::copy(ids->begin(), ids->end(), result->inserter());
            }
        }
    }

    return result;
}

namespace
{
    struct DepCollector :
        ConstVisitor<DependencySpecTree>,
        ConstVisitor<DependencySpecTree>::VisitConstSequence<DepCollector, AllDepSpec>,
        ConstVisitor<DependencySpecTree>::VisitConstSequence<DepCollector, AnyDepSpec>
    {
        using ConstVisitor<DependencySpecTree>::VisitConstSequence<DepCollector, AllDepSpec>::visit_sequence;

        const Environment * const env;
        const tr1::shared_ptr<const PackageID> pkg;
        tr1::shared_ptr<DepListEntryTags> matches;
        tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > conditions;


        DepCollector(const Environment * const ee, const tr1::shared_ptr<const PackageID> & e) :
            env(ee),
            pkg(e),
            matches(new DepListEntryTags::Concrete),
            conditions(tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >(
                           new ConstTreeSequence<DependencySpecTree, AllDepSpec>(
                               tr1::shared_ptr<AllDepSpec>(new AllDepSpec))))
        {
        }

        void visit_leaf(const PackageDepSpec & a)
        {
            Save<tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
                &conditions, ConditionTracker(conditions).add_condition(a));

            tr1::shared_ptr<const PackageIDSequence> m(env->package_database()->query(
                        query::Matches(a) & query::InstalledAtRoot(env->root()), qo_order_by_version));
            for (PackageIDSequence::Iterator it = m->begin(), it_end = m->end();
                 it_end != it; ++it)
                matches->insert(DepTagEntry(tr1::shared_ptr<const DepTag>(new DependencyDepTag(*it, a, conditions)), 0));
        }

        void visit_sequence(const AnyDepSpec & a,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            Save<tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
                &conditions, ConditionTracker(conditions).add_condition(a));

            std::for_each(cur, end, accept_visitor(*this));
        }

        void visit_sequence(const UseDepSpec & u,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            Save<tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
                &conditions, ConditionTracker(conditions).add_condition(u));

            if (env->query_use(UseFlagName(u.flag()), *pkg) ^ u.inverse())
                std::for_each(cur, end, accept_visitor(*this));
        }

        void visit_leaf(const BlockDepSpec &)
        {
        }
    };
}

tr1::shared_ptr<const PackageIDSet>
UninstallList::collect_depped_upon(tr1::shared_ptr<const PackageIDSet> targets) const
{
    Context context("When collecting depended upon packages:");

    tr1::shared_ptr<PackageIDSet> result(new PackageIDSet::Concrete);

    for (PackageIDSet::Iterator i(targets->begin()), i_end(targets->end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When collecting depended upon packages for '" + stringify(**i) + "':");

        DepCollectorCache::const_iterator cache(_imp->dep_collector_cache.find(*i));

        if (cache == _imp->dep_collector_cache.end())
        {
            DepCollector c(_imp->env, *i);
            if ((*i)->build_dependencies_key())
                (*i)->build_dependencies_key()->value()->accept(c);
            if ((*i)->run_dependencies_key())
                (*i)->run_dependencies_key()->value()->accept(c);
            if ((*i)->post_dependencies_key())
                (*i)->post_dependencies_key()->value()->accept(c);
            if ((*i)->suggested_dependencies_key())
                (*i)->suggested_dependencies_key()->value()->accept(c);

            cache = _imp->dep_collector_cache.insert(std::make_pair(*i,
                        tr1::shared_ptr<const DepListEntryTags>(c.matches))).first;
        }

        for (DepListEntryTags::Iterator it(cache->second->begin()), it_end(cache->second->end());
             it_end != it; ++it)
            result->insert(tr1::static_pointer_cast<const DependencyDepTag>(it->tag)->package_id());
    }

    return result;
}

void
UninstallList::add_unused_dependencies()
{
    Context context("When adding unused dependencies:");

    bool added(true);
    tr1::shared_ptr<const PackageIDSet> everything(collect_all_installed());
    while (added)
    {
        added = false;

        /* find packages that're depped upon by anything in our uninstall list */
        tr1::shared_ptr<PackageIDSet> uninstall_list_targets(new PackageIDSet::Concrete);
        for (std::list<UninstallListEntry>::const_iterator i(_imp->uninstall_list.begin()),
                i_end(_imp->uninstall_list.end()) ; i != i_end ; ++i)
            uninstall_list_targets->insert(i->package_id);

        tr1::shared_ptr<const PackageIDSet> depped_upon_list(collect_depped_upon(uninstall_list_targets));

        /* find packages that're depped upon by anything not in our uninstall list */
        tr1::shared_ptr<PackageIDSet> everything_except_uninstall_list_targets(
                new PackageIDSet::Concrete);
        std::set_difference(everything->begin(), everything->end(),
                uninstall_list_targets->begin(), uninstall_list_targets->end(),
                everything_except_uninstall_list_targets->inserter(),
                PackageIDSetComparator());

        Log::get_instance()->message(ll_debug, lc_context, "everything_except_uninstall_list_targets is '"
                + join(indirect_iterator(everything_except_uninstall_list_targets->begin()),
                    indirect_iterator(everything_except_uninstall_list_targets->end()), " ") + "'");

        tr1::shared_ptr<const PackageIDSet> depped_upon_not_list(
                collect_depped_upon(everything_except_uninstall_list_targets));

        /* find unused dependencies */
        tr1::shared_ptr<PackageIDSet> unused_dependencies(new PackageIDSet::Concrete);
        std::set_difference(depped_upon_list->begin(), depped_upon_list->end(),
                depped_upon_not_list->begin(), depped_upon_not_list->end(), unused_dependencies->inserter(),
                PackageIDSetComparator());

        /* if any of them aren't already on the list, and aren't in world, add them and recurse */
        tr1::shared_ptr<SetSpecTree::ConstItem> world(_imp->env->set(SetName("world")));
        for (PackageIDSet::Iterator i(unused_dependencies->begin()),
                i_end(unused_dependencies->end()) ; i != i_end ; ++i)
        {
            if (match_package_in_set(*_imp->env, *world, **i))
                continue;

            if (_imp->uninstall_list.end() != std::find_if(_imp->uninstall_list.begin(),
                        _imp->uninstall_list.end(), MatchUninstallListEntry(*i)))
                continue;

            add_package(*i, tr1::shared_ptr<DepTag>());
            added = true;
        }
    }
}

void
UninstallList::add_dependencies(const PackageID & e)
{
    Context context("When adding things that depend upon '" + stringify(e) + "':");

    tr1::shared_ptr<const PackageIDSet> everything(collect_all_installed());
    for (PackageIDSet::Iterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
    {
        Context local_context("When seeing whether '" + stringify(**i) + "' has a dep:");

        DepCollectorCache::const_iterator cache(_imp->dep_collector_cache.find(*i));

        if (cache == _imp->dep_collector_cache.end())
        {
            DepCollector c(_imp->env, *i);
            if ((*i)->build_dependencies_key())
                (*i)->build_dependencies_key()->value()->accept(c);
            if ((*i)->run_dependencies_key())
                (*i)->run_dependencies_key()->value()->accept(c);
            if ((*i)->post_dependencies_key())
                (*i)->post_dependencies_key()->value()->accept(c);
            if ((*i)->suggested_dependencies_key())
                (*i)->suggested_dependencies_key()->value()->accept(c);
            cache = _imp->dep_collector_cache.insert(std::make_pair(*i,
                        tr1::shared_ptr<const DepListEntryTags>(c.matches))).first;
        }

        bool logged(false);
        for (DepListEntryTags::Iterator it(cache->second->begin()), it_end(cache->second->end());
             it_end != it; ++it)
        {
            tr1::shared_ptr<const DependencyDepTag> tag(tr1::static_pointer_cast<const DependencyDepTag>(it->tag));
            if (*tag->package_id() == e)
            {
                if (! logged)
                {
                    Log::get_instance()->message(ll_debug, lc_context) << "Adding '" << **i <<
                            "' because it depends upon '" << e << "'";
                    logged = true;
                }
                add(*i, tr1::shared_ptr<DependencyDepTag>(
                        new DependencyDepTag(tag->package_id(), *tag->dependency(), tag->conditions())));
            }
        }

    }
}

tr1::shared_ptr<const PackageIDSet>
UninstallList::collect_world() const
{
    Context local_context("When collecting world packages:");

    tr1::shared_ptr<PackageIDSet> result(new PackageIDSet::Concrete);
    tr1::shared_ptr<const PackageIDSet> everything(collect_all_installed());

    tr1::shared_ptr<SetSpecTree::ConstItem> world(_imp->env->set(SetName("world")));
    for (PackageIDSet::Iterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
        if (match_package_in_set(*_imp->env, *world, **i))
            result->insert(*i);

    return result;
}

