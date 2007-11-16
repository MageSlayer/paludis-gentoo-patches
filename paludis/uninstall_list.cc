/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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
#include <paludis/condition_tracker.hh>
#include <paludis/environment.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/query.hh>
#include <paludis/metadata_key.hh>
#include <paludis/dep_tag.hh>
#include <list>
#include <algorithm>
#include <set>

using namespace paludis;

#include <paludis/uninstall_list-se.cc>
#include <paludis/uninstall_list-sr.cc>

typedef MakeHashedMap<tr1::shared_ptr<const PackageID>, tr1::shared_ptr<const DepListEntryTags> >::Type DepCollectorCache;

template class Set<tr1::shared_ptr<DepTag> >;
template class WrappedForwardIterator<Set<tr1::shared_ptr<DepTag> >::ConstIteratorTag, const tr1::shared_ptr<DepTag> >;
template class WrappedOutputIterator<Set<tr1::shared_ptr<DepTag> >::InserterTag, tr1::shared_ptr<DepTag> >;

namespace paludis
{
    template<>
    struct Implementation<UninstallList>
    {
        const Environment * const env;
        UninstallListOptions options;
        std::list<UninstallListEntry> uninstall_list;

        mutable Mutex dep_collector_cache_mutex;
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
    real_add(e, t, false);
}

void
UninstallList::real_add(const tr1::shared_ptr<const PackageID> & e, tr1::shared_ptr<DepTag> t,
        const bool error)
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

    if ((! error) || (! e->virtual_for_key()))
        add_package(e, t, error ? ulk_required : (e->virtual_for_key() ? ulk_virtual : ulk_package));

    if (! error)
    {
        /* don't recurse errors, it gets horrid */
        if (_imp->options.with_dependencies_included)
            add_dependencies(*e, false);
        else if (_imp->options.with_dependencies_as_errors)
            add_dependencies(*e, true);
    }

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
        world_plus_deps(new PackageIDSet),
        unused(new PackageIDSet);

    std::copy(world->begin(), world->end(), world_plus_deps->inserter());

    std::size_t old_size(0);
    while (old_size != world_plus_deps->size())
    {
        old_size = world_plus_deps->size();
        tr1::shared_ptr<const PackageIDSet> new_world_deps(collect_depped_upon(world_plus_deps));
        std::copy(new_world_deps->begin(), new_world_deps->end(), world_plus_deps->inserter());
    }

    std::set_difference(everything->begin(), everything->end(),
            world_plus_deps->begin(), world_plus_deps->end(), unused->inserter(),
            PackageIDSetComparator());

    for (PackageIDSet::ConstIterator i(unused->begin()), i_end(unused->end()) ; i != i_end ; ++i)
        add_package(*i, tr1::shared_ptr<DepTag>(), (*i)->virtual_for_key() ? ulk_virtual : ulk_package);
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

UninstallListOptions::UninstallListOptions() :
    with_unused_dependencies(false),
    with_dependencies_included(false),
    with_dependencies_as_errors(false)
{
}

void
UninstallList::add_package(const tr1::shared_ptr<const PackageID> & e, tr1::shared_ptr<DepTag> t,
        const UninstallListEntryKind k)
{
    Context context("When adding package '" + stringify(*e) + "' to the uninstall list:");

    std::list<UninstallListEntry>::iterator i(_imp->uninstall_list.insert(
                _imp->uninstall_list.end(), UninstallListEntry(UninstallListEntry::create()
                    .package_id(e)
                    .tags(make_shared_ptr(new Set<tr1::shared_ptr<DepTag> >))
                    .kind(k))));

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

    tr1::shared_ptr<PackageIDSet> result(new PackageIDSet);
    for (PackageDatabase::RepositoryConstIterator i(_imp->env->package_database()->begin_repositories()),
            i_end(_imp->env->package_database()->end_repositories()) ; i != i_end ; ++i)
    {
        if (! (*i)->installed_root_key())
            continue;

        tr1::shared_ptr<const CategoryNamePartSet> cats((*i)->category_names());
        for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                c != c_end ; ++c)
        {
            tr1::shared_ptr<const QualifiedPackageNameSet> pkgs((*i)->package_names(*c));
            for (QualifiedPackageNameSet::ConstIterator p(pkgs->begin()), p_end(pkgs->end()) ;
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
        std::set<SetName> recursing_sets;

        DepCollector(const Environment * const ee, const tr1::shared_ptr<const PackageID> & e) :
            env(ee),
            pkg(e),
            matches(new DepListEntryTags),
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
            for (PackageIDSequence::ConstIterator it = m->begin(), it_end = m->end();
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

        void visit_leaf(const DependencyLabelsDepSpec &)
        {
        }

        void visit_leaf(const NamedSetDepSpec & s)
        {
            Context context("When expanding named set '" + stringify(s) + "':");

            tr1::shared_ptr<const SetSpecTree::ConstItem> set(env->set(s.name()));

            if (! set)
            {
                Log::get_instance()->message(ll_warning, lc_context) << "Unknown set '" << s.name() << "'";
                return;
            }

            if (! recursing_sets.insert(s.name()).second)
            {
                Log::get_instance()->message(ll_warning, lc_context) << "Recursively defined set '" << s.name() << "'";
                return;
            }

            set->accept(*this);

            recursing_sets.erase(s.name());
        }
    };
}

tr1::shared_ptr<const PackageIDSet>
UninstallList::collect_depped_upon(tr1::shared_ptr<const PackageIDSet> targets) const
{
    Context context("When collecting depended upon packages:");

    tr1::shared_ptr<PackageIDSet> result(new PackageIDSet);

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

        for (DepListEntryTags::ConstIterator it(cache->second->begin()), it_end(cache->second->end());
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

        /* find packages that're depped upon by anything in our uninstall list, excluding error
         * packages */
        tr1::shared_ptr<PackageIDSet> uninstall_list_targets(new PackageIDSet);
        for (std::list<UninstallListEntry>::const_iterator i(_imp->uninstall_list.begin()),
                i_end(_imp->uninstall_list.end()) ; i != i_end ; ++i)
            if (i->kind == ulk_package || i->kind == ulk_virtual)
                uninstall_list_targets->insert(i->package_id);

        tr1::shared_ptr<const PackageIDSet> depped_upon_list(collect_depped_upon(uninstall_list_targets));

        /* find packages that're depped upon by anything not in our uninstall list */
        tr1::shared_ptr<PackageIDSet> everything_except_uninstall_list_targets(
                new PackageIDSet);
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
        tr1::shared_ptr<PackageIDSet> unused_dependencies(new PackageIDSet);
        std::set_difference(depped_upon_list->begin(), depped_upon_list->end(),
                depped_upon_not_list->begin(), depped_upon_not_list->end(), unused_dependencies->inserter(),
                PackageIDSetComparator());

        /* if any of them aren't already on the list, and aren't in world or ununused, add them and recurse */
        tr1::shared_ptr<SetSpecTree::ConstItem> world(_imp->env->set(SetName("world"))),
            ununused(_imp->env->set(SetName("ununused")));
        for (PackageIDSet::ConstIterator i(unused_dependencies->begin()),
                i_end(unused_dependencies->end()) ; i != i_end ; ++i)
        {
            if (match_package_in_set(*_imp->env, *world, **i))
                continue;
            if (ununused && match_package_in_set(*_imp->env, *ununused, **i))
                continue;

            if (_imp->uninstall_list.end() != std::find_if(_imp->uninstall_list.begin(),
                        _imp->uninstall_list.end(), MatchUninstallListEntry(*i)))
                continue;

            add_package(*i, tr1::shared_ptr<DepTag>(), (*i)->virtual_for_key() ? ulk_virtual : ulk_package);
            added = true;
        }
    }
}

void
UninstallList::add_dependencies(const PackageID & e, const bool error)
{
    Context context("When adding things that depend upon '" + stringify(e) + "':");

    tr1::shared_ptr<const PackageIDSet> everything(collect_all_installed());

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
        for (DepListEntryTags::ConstIterator it(cache->second->begin()), it_end(cache->second->end());
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
                real_add(*i, tr1::shared_ptr<DependencyDepTag>(
                        new DependencyDepTag(tag->package_id(), *tag->dependency(), tag->conditions())), error);
            }
        }

    }
}

tr1::shared_ptr<const PackageIDSet>
UninstallList::collect_world() const
{
    Context local_context("When collecting world packages:");

    tr1::shared_ptr<PackageIDSet> result(new PackageIDSet);
    tr1::shared_ptr<const PackageIDSet> everything(collect_all_installed());

    tr1::shared_ptr<SetSpecTree::ConstItem> world(_imp->env->set(SetName("world"))),
        ununused(_imp->env->set(SetName("ununused")));
    for (PackageIDSet::ConstIterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
    {
        if (match_package_in_set(*_imp->env, *world, **i))
            result->insert(*i);
        else if (ununused && match_package_in_set(*_imp->env, *ununused, **i))
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
            switch (e.kind)
            {
                case ulk_virtual:
                case ulk_package:
                    return false;

                case ulk_required:
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

