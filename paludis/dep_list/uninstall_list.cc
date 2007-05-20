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
#include <paludis/environment.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/match_package.hh>
#include <paludis/version_metadata.hh>
#include <paludis/package_database.hh>
#include <list>
#include <algorithm>

namespace paludis
{
    template<>
    class CRCHash<PackageDatabaseEntry> :
        public CRCHash<std::string>
    {
        public:
            /// Hash function.
            std::size_t operator() (const PackageDatabaseEntry & val) const
            {
                return CRCHash<std::string>::operator() (stringify(val));
            }

#if (! PALUDIS_HAVE_TR1_HASHES) && (! PALUDIS_HAVE_EXT_HASHES)
            bool operator() (const PackageDatabaseEntry & lhs,
                    const PackageDatabaseEntry & rhs) const
            {
                return stringify(lhs) < stringify(rhs);
            }
#endif
    };

    template<>
    struct Implementation<UninstallList>
    {
        const Environment * const env;
        UninstallListOptions options;
        std::list<UninstallListEntry> uninstall_list;

        mutable MakeHashedMap<PackageDatabaseEntry, tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> >::Type
            dep_collector_cache;

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
        PackageDatabaseEntry e;

        MatchUninstallListEntry(const PackageDatabaseEntry & ee) :
            e(ee)
        {
        }

        bool operator() (const UninstallListEntry & f)
        {
            return f.package == e;
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
UninstallList::add(const PackageDatabaseEntry & e, const PackageDatabaseEntry * const t)
{
    std::list<UninstallListEntry>::iterator i;
    if (_imp->uninstall_list.end() != ((i = std::find_if(_imp->uninstall_list.begin(),
                        _imp->uninstall_list.end(), MatchUninstallListEntry(e)))))
    {
        if (t)
            i->tags->insert(tr1::shared_ptr<DepTag>(new DependencyDepTag(*t)));

        return;
    }

    Context context("When adding '" + stringify(e) + "' to the uninstall list:");

    add_package(e, t);

    if (_imp->options.with_dependencies)
        add_dependencies(e);

    move_package_to_end(e);

    if (_imp->options.with_unused_dependencies)
        add_unused_dependencies();
}

void
UninstallList::add_unused()
{
    Context context("When finding unused packages:");

    tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> world(collect_world()),
        everything(collect_all_installed());

    tr1::shared_ptr<ArbitrarilyOrderedPackageDatabaseEntryCollection>
        world_plus_deps(new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete),
        unused(new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);

    world_plus_deps->insert(world->begin(), world->end());

    std::size_t old_size(0);
    while (old_size != world_plus_deps->size())
    {
        old_size = world_plus_deps->size();
        tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> new_world_deps(collect_depped_upon(world_plus_deps));
        world_plus_deps->insert(new_world_deps->begin(), new_world_deps->end());
    }

    std::set_difference(everything->begin(), everything->end(),
            world_plus_deps->begin(), world_plus_deps->end(), unused->inserter(),
            ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator());

    for (PackageDatabaseEntryCollection::Iterator i(unused->begin()),
            i_end(unused->end()) ; i != i_end ; ++i)
        add_package(*i, 0);
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
UninstallList::add_package(const PackageDatabaseEntry & e, const PackageDatabaseEntry * t)
{
    Context context("When adding package '" + stringify(e) + "' to the uninstall list:");

    tr1::shared_ptr<const VersionMetadata> m(_imp->env->package_database()->fetch_repository(
                e.repository)->version_metadata(e.name, e.version));

    std::list<UninstallListEntry>::iterator i(_imp->uninstall_list.insert(
                _imp->uninstall_list.end(), UninstallListEntry(
                    e, m->virtual_interface, tr1::shared_ptr<SortedCollection<tr1::shared_ptr<DepTag> > >(
                        new SortedCollection<tr1::shared_ptr<DepTag> >::Concrete))));
    if (t)
        i->tags->insert(tr1::shared_ptr<DepTag>(new DependencyDepTag(*t)));
}

void
UninstallList::move_package_to_end(const PackageDatabaseEntry & e)
{
    Context context("When removing package '" + stringify(e) + "' from the uninstall list:");

    std::list<UninstallListEntry>::iterator i(std::find_if(_imp->uninstall_list.begin(),
                _imp->uninstall_list.end(), MatchUninstallListEntry(e)));
    if (_imp->uninstall_list.end() != i)
        _imp->uninstall_list.splice(_imp->uninstall_list.end(), _imp->uninstall_list, i);
}

tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection>
UninstallList::collect_all_installed() const
{
    Context context("When collecting all installed packages:");

    tr1::shared_ptr<ArbitrarilyOrderedPackageDatabaseEntryCollection> result(
            new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);
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
                tr1::shared_ptr<const VersionSpecCollection> vers((*i)->version_specs(*p));
                for (VersionSpecCollection::Iterator v(vers->begin()), v_end(vers->end()) ;
                        v != v_end ; ++v)
                    result->insert(PackageDatabaseEntry(*p, *v, (*i)->name()));
            }
        }
    }

    return result;
}

namespace
{
    struct DepCollector :
        DepSpecVisitorTypes::ConstVisitor,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<DepCollector, AllDepSpec>,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<DepCollector, AnyDepSpec>
    {
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<DepCollector, AllDepSpec>::visit;
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<DepCollector, AnyDepSpec>::visit;

        const Environment * const env;
        const PackageDatabaseEntry pkg;
        tr1::shared_ptr<ArbitrarilyOrderedPackageDatabaseEntryCollection> matches;

        DepCollector(const Environment * const ee, const PackageDatabaseEntry & e) :
            env(ee),
            pkg(e),
            matches(new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete)
        {
        }

        void visit(const PackageDepSpec * const a)
        {
            tr1::shared_ptr<const PackageDatabaseEntryCollection> m(env->package_database()->query(
                        *a, is_installed_only, qo_order_by_version));
            matches->insert(m->begin(), m->end());
        }

        void visit(const UseDepSpec * const u)
        {
            if (env->query_use(UseFlagName(u->flag()), pkg) ^ u->inverse())
                std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const BlockDepSpec * const)
        {
        }

        void visit(const PlainTextDepSpec * const) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Got PlainTextDepSpec?");
        }
    };
}

tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection>
UninstallList::collect_depped_upon(tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> targets) const
{
    Context context("When collecting depended upon packages:");

    tr1::shared_ptr<ArbitrarilyOrderedPackageDatabaseEntryCollection> result(
            new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);

    for (PackageDatabaseEntryCollection::Iterator i(targets->begin()), i_end(targets->end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When collecting depended upon packages for '" + stringify(*i) + "':");

        MakeHashedMap<PackageDatabaseEntry,
            tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> >::Type::const_iterator
                cache(_imp->dep_collector_cache.find(*i));

        if (cache == _imp->dep_collector_cache.end())
        {
            DepCollector c(_imp->env, *i);
            tr1::shared_ptr<const VersionMetadata> metadata(_imp->env->package_database()->fetch_repository(
                        i->repository)->version_metadata(i->name, i->version));
            if (metadata->deps_interface)
            {
                metadata->deps_interface->build_depend()->accept(&c);
                metadata->deps_interface->run_depend()->accept(&c);
                metadata->deps_interface->post_depend()->accept(&c);
                metadata->deps_interface->suggested_depend()->accept(&c);
            }
            cache = _imp->dep_collector_cache.insert(std::make_pair(*i,
                        tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection>(c.matches))).first;
        }

        result->insert(cache->second->begin(), cache->second->end());
    }

    return result;
}

void
UninstallList::add_unused_dependencies()
{
    Context context("When adding unused dependencies:");

    bool added(true);
    tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> everything(collect_all_installed());
    while (added)
    {
        added = false;

        /* find packages that're depped upon by anything in our uninstall list */
        tr1::shared_ptr<ArbitrarilyOrderedPackageDatabaseEntryCollection> uninstall_list_targets(
                new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);
        for (std::list<UninstallListEntry>::const_iterator i(_imp->uninstall_list.begin()),
                i_end(_imp->uninstall_list.end()) ; i != i_end ; ++i)
            uninstall_list_targets->insert(i->package);

        tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> depped_upon_list(
                collect_depped_upon(uninstall_list_targets));

        /* find packages that're depped upon by anything not in our uninstall list */
        tr1::shared_ptr<ArbitrarilyOrderedPackageDatabaseEntryCollection> everything_except_uninstall_list_targets(
                new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);
        std::set_difference(everything->begin(), everything->end(),
                uninstall_list_targets->begin(), uninstall_list_targets->end(),
                everything_except_uninstall_list_targets->inserter(),
                ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator());

        Log::get_instance()->message(ll_debug, lc_context, "everything_except_uninstall_list_targets is '"
                + join(everything_except_uninstall_list_targets->begin(),
                    everything_except_uninstall_list_targets->end(), " ") + "'");

        tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> depped_upon_not_list(
                collect_depped_upon(everything_except_uninstall_list_targets));

        /* find unused dependencies */
        tr1::shared_ptr<ArbitrarilyOrderedPackageDatabaseEntryCollection> unused_dependencies(
                new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);
        std::set_difference(depped_upon_list->begin(), depped_upon_list->end(),
                depped_upon_not_list->begin(), depped_upon_not_list->end(), unused_dependencies->inserter(),
                ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator());

        /* if any of them aren't already on the list, and aren't in world, add them and recurse */
        tr1::shared_ptr<DepSpec> world(_imp->env->set(SetName("world")));
        for (PackageDatabaseEntryCollection::Iterator i(unused_dependencies->begin()),
                i_end(unused_dependencies->end()) ; i != i_end ; ++i)
        {
            if (match_package_in_heirarchy(*_imp->env, *world, *i))
                continue;

            if (_imp->uninstall_list.end() != std::find_if(_imp->uninstall_list.begin(),
                        _imp->uninstall_list.end(), MatchUninstallListEntry(*i)))
                continue;

            add_package(*i, 0);
            added = true;
        }
    }
}

void
UninstallList::add_dependencies(const PackageDatabaseEntry & e)
{
    Context context("When adding things that depend upon '" + stringify(e) + "':");

    tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> everything(collect_all_installed());
    for (PackageDatabaseEntryCollection::Iterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
    {
        Context local_context("When seeing whether '" + stringify(*i) + "' has a dep:");

        MakeHashedMap<PackageDatabaseEntry,
            tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> >::Type::const_iterator
                cache(_imp->dep_collector_cache.find(*i));

        if (cache == _imp->dep_collector_cache.end())
        {
            DepCollector c(_imp->env, *i);
            tr1::shared_ptr<const VersionMetadata> metadata(_imp->env->package_database()->fetch_repository(
                        i->repository)->version_metadata(i->name, i->version));
            if (metadata->deps_interface)
            {
                metadata->deps_interface->build_depend()->accept(&c);
                metadata->deps_interface->run_depend()->accept(&c);
                metadata->deps_interface->post_depend()->accept(&c);
                metadata->deps_interface->suggested_depend()->accept(&c);
            }
            cache = _imp->dep_collector_cache.insert(std::make_pair(*i,
                        tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection>(c.matches))).first;
        }

        if (cache->second->end() == cache->second->find(e))
            continue;

        Log::get_instance()->message(ll_debug, lc_context, "Adding '" + stringify(*i) +
                "' because it depends upon '" + stringify(e) + "'");

        add(*i, &e);
    }
}

tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection>
UninstallList::collect_world() const
{
    Context local_context("When collecting world packages:");

    tr1::shared_ptr<ArbitrarilyOrderedPackageDatabaseEntryCollection> result(
            new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);
    tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> everything(collect_all_installed());

    tr1::shared_ptr<DepSpec> world(_imp->env->set(SetName("world")));
    for (PackageDatabaseEntryCollection::Iterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
        if (match_package_in_heirarchy(*_imp->env, *world, *i))
            result->insert(*i);

    return result;
}

