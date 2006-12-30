/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/hashed_containers.hh>
#include <paludis/match_package.hh>
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
    struct Implementation<UninstallList> :
        InternalCounted<Implementation<UninstallList> >
    {
        const Environment * const env;
        UninstallListOptions options;
        std::list<UninstallListEntry> uninstall_list;

        mutable MakeHashedMap<PackageDatabaseEntry, ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer>::Type
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

    struct IsWorld :
        DepAtomVisitorTypes::ConstVisitor,
        std::unary_function<PackageDatabaseEntry, bool>
    {
        const Environment * const env;
        DepAtom::ConstPointer world;
        const PackageDatabaseEntry * dbe;
        bool matched;

        IsWorld(const Environment * const e) :
            env(e),
            world(e->package_set(SetName("world"))),
            matched(false)
        {
        }

        bool operator() (const PackageDatabaseEntry & e)
        {
            dbe = &e;
            matched = false;
            world->accept(this);
            return matched;
        }

        void visit(const AllDepAtom * const a)
        {
            if (matched)
                return;

            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const PackageDepAtom * const a)
        {
            if (matched)
                return;

            if (match_package(env, a, *dbe))
                matched = true;
        }

        void visit(const UseDepAtom * const u)
        {
            if (matched)
                return;

            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * const a)
        {
            if (matched)
                return;

            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * const)
        {
        }

        void visit(const PlainTextDepAtom * const) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Got PlainTextDepAtom?");
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
            i->tags->insert(DepTag::Pointer(new DependencyDepTag(*t)));

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

    ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer world(collect_world()),
        everything(collect_all_installed());

    ArbitrarilyOrderedPackageDatabaseEntryCollection::Pointer
        world_plus_deps(new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete),
        unused(new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);

    world_plus_deps->insert(world->begin(), world->end());

    std::size_t old_size(0);
    while (old_size != world_plus_deps->size())
    {
        old_size = world_plus_deps->size();
        ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer new_world_deps(collect_depped_upon(world_plus_deps));
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

    VersionMetadata::ConstPointer m(_imp->env->package_database()->fetch_repository(
                e.repository)->version_metadata(e.name, e.version));

    std::list<UninstallListEntry>::iterator i(_imp->uninstall_list.insert(
                _imp->uninstall_list.end(), UninstallListEntry(
                    e, m->get_virtual_interface(), SortedCollection<DepTag::Pointer>::Pointer(
                        new SortedCollection<DepTag::Pointer>::Concrete))));
    if (t)
        i->tags->insert(DepTag::Pointer(new DependencyDepTag(*t)));
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

ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer
UninstallList::collect_all_installed() const
{
    Context context("When collecting all installed packages:");

    ArbitrarilyOrderedPackageDatabaseEntryCollection::Pointer result(
            new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);
    for (PackageDatabase::RepositoryIterator i(_imp->env->package_database()->begin_repositories()),
            i_end(_imp->env->package_database()->end_repositories()) ; i != i_end ; ++i)
    {
        if (! (*i)->installed_interface)
            continue;

        CategoryNamePartCollection::ConstPointer cats((*i)->category_names());
        for (CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end()) ;
                c != c_end ; ++c)
        {
            QualifiedPackageNameCollection::ConstPointer pkgs((*i)->package_names(*c));
            for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                    p != p_end ; ++p)
            {
                VersionSpecCollection::ConstPointer vers((*i)->version_specs(*p));
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
        DepAtomVisitorTypes::ConstVisitor
    {
        const Environment * const env;
        const PackageDatabaseEntry pkg;
        ArbitrarilyOrderedPackageDatabaseEntryCollection::Pointer matches;

        DepCollector(const Environment * const ee, const PackageDatabaseEntry & e) :
            env(ee),
            pkg(e),
            matches(new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete)
        {
        }

        void visit(const AllDepAtom * const a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const PackageDepAtom * const a)
        {
            PackageDatabaseEntryCollection::ConstPointer m(env->package_database()->query(
                        *a, is_installed_only));
            matches->insert(m->begin(), m->end());
        }

        void visit(const UseDepAtom * const u)
        {
            if (env->query_use(UseFlagName(u->flag()), &pkg) ^ u->inverse())
                std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * const a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * const)
        {
        }

        void visit(const PlainTextDepAtom * const) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Got PlainTextDepAtom?");
        }
    };
}

ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer
UninstallList::collect_depped_upon(ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer targets) const
{
    Context context("When collecting depended upon packages:");

    ArbitrarilyOrderedPackageDatabaseEntryCollection::Pointer result(
            new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);

    for (PackageDatabaseEntryCollection::Iterator i(targets->begin()), i_end(targets->end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When collecting depended upon packages for '" + stringify(*i) + "':");

        MakeHashedMap<PackageDatabaseEntry, ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer>::Type::const_iterator
            cache(_imp->dep_collector_cache.find(*i));
        if (cache == _imp->dep_collector_cache.end())
        {
            DepCollector c(_imp->env, *i);
            VersionMetadata::ConstPointer metadata(_imp->env->package_database()->fetch_repository(
                        i->repository)->version_metadata(i->name, i->version));
            metadata->deps.build_depend()->accept(&c);
            metadata->deps.run_depend()->accept(&c);
            metadata->deps.post_depend()->accept(&c);
            cache = _imp->dep_collector_cache.insert(std::make_pair(*i,
                        ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer(c.matches))).first;
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
    ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer everything(collect_all_installed());
    while (added)
    {
        added = false;

        /* find packages that're depped upon by anything in our uninstall list */
        ArbitrarilyOrderedPackageDatabaseEntryCollection::Pointer uninstall_list_targets(
                new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);
        for (std::list<UninstallListEntry>::const_iterator i(_imp->uninstall_list.begin()),
                i_end(_imp->uninstall_list.end()) ; i != i_end ; ++i)
            uninstall_list_targets->insert(i->package);

        ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer depped_upon_list(
                collect_depped_upon(uninstall_list_targets));

        /* find packages that're depped upon by anything not in our uninstall list */
        ArbitrarilyOrderedPackageDatabaseEntryCollection::Pointer everything_except_uninstall_list_targets(
                new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);
        std::set_difference(everything->begin(), everything->end(),
                uninstall_list_targets->begin(), uninstall_list_targets->end(),
                everything_except_uninstall_list_targets->inserter(),
                ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator());

        Log::get_instance()->message(ll_debug, lc_context, "everything_except_uninstall_list_targets is '"
                + join(everything_except_uninstall_list_targets->begin(),
                    everything_except_uninstall_list_targets->end(), " ") + "'");

        ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer depped_upon_not_list(
                collect_depped_upon(everything_except_uninstall_list_targets));

        /* find unused dependencies */
        ArbitrarilyOrderedPackageDatabaseEntryCollection::Pointer unused_dependencies(
                new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);
        std::set_difference(depped_upon_list->begin(), depped_upon_list->end(),
                depped_upon_not_list->begin(), depped_upon_not_list->end(), unused_dependencies->inserter(),
                ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator());

        /* if any of them aren't already on the list, and aren't in world, add them and recurse */
        IsWorld w(_imp->env);
        for (PackageDatabaseEntryCollection::Iterator i(unused_dependencies->begin()),
                i_end(unused_dependencies->end()) ; i != i_end ; ++i)
        {
            if (w(*i))
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

    ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer everything(collect_all_installed());
    for (PackageDatabaseEntryCollection::Iterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
    {
        Context local_context("When seeing whether '" + stringify(*i) + "' has a dep:");

        MakeHashedMap<PackageDatabaseEntry, ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer>::Type::const_iterator
            cache(_imp->dep_collector_cache.find(*i));
        if (cache == _imp->dep_collector_cache.end())
        {
            DepCollector c(_imp->env, *i);
            VersionMetadata::ConstPointer metadata(_imp->env->package_database()->fetch_repository(
                        i->repository)->version_metadata(i->name, i->version));
            metadata->deps.build_depend()->accept(&c);
            metadata->deps.run_depend()->accept(&c);
            metadata->deps.post_depend()->accept(&c);
            cache = _imp->dep_collector_cache.insert(std::make_pair(*i,
                        ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer(c.matches))).first;
        }

        if (cache->second->end() == cache->second->find(e))
            continue;

        Log::get_instance()->message(ll_debug, lc_context, "Adding '" + stringify(*i) +
                "' because it depends upon '" + stringify(e) + "'");

        add(*i, &e);
    }
}

ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer
UninstallList::collect_world() const
{
    Context local_context("When collecting world packages:");

    ArbitrarilyOrderedPackageDatabaseEntryCollection::Pointer result(
            new ArbitrarilyOrderedPackageDatabaseEntryCollection::Concrete);
    ArbitrarilyOrderedPackageDatabaseEntryCollection::ConstPointer everything(collect_all_installed());

    IsWorld w(_imp->env);
    for (PackageDatabaseEntryCollection::Iterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
        if (w(*i))
            result->insert(*i);

    return result;
}

