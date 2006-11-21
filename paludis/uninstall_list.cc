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

#include <paludis/uninstall_list-sr.cc>
#include <paludis/environment.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <list>
#include <algorithm>

namespace paludis
{
    template<>
    struct Implementation<UninstallList> :
        InternalCounted<Implementation<UninstallList> >
    {
        const Environment * const env;
        UninstallListOptions options;
        std::list<UninstallListEntry> uninstall_list;

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
UninstallList::add(const PackageDatabaseEntry & e)
{
    if (_imp->uninstall_list.end() != std::find_if(_imp->uninstall_list.begin(),
                _imp->uninstall_list.end(), MatchUninstallListEntry(e)))
        return;

    Context context("When adding '" + stringify(e) + "' to the uninstall list:");

    add_package(e);

    if (_imp->options.with_dependencies)
        add_dependencies(e);

    if (_imp->options.with_unused_dependencies)
        add_unused_dependencies();
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
UninstallList::add_package(const PackageDatabaseEntry & e)
{
    Context context("When adding package '" + stringify(e) + "' to the uninstall list:");

    if (! _imp->env->package_database()->fetch_repository(e.repository)->has_version(e.name, e.version))
        throw InternalError(PALUDIS_HERE, "Trying to add '" + stringify(e) +
                "' to UninstallList but has_version failed");

    _imp->uninstall_list.push_back(e);
}

PackageDatabaseEntryCollection::ConstPointer
UninstallList::collect_all_installed() const
{
    Context context("When collecting all installed packages:");

    PackageDatabaseEntryCollection::Pointer result(new PackageDatabaseEntryCollection::Concrete);
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
        PackageDatabaseEntryCollection::Pointer matches;

        DepCollector(const Environment * const ee, const PackageDatabaseEntry & e) :
            env(ee),
            pkg(e),
            matches(new PackageDatabaseEntryCollection::Concrete)
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

PackageDatabaseEntryCollection::ConstPointer
UninstallList::collect_depped_upon(PackageDatabaseEntryCollection::ConstPointer targets) const
{
    Context context("When collecting depended upon packages:");

    PackageDatabaseEntryCollection::Pointer result(new PackageDatabaseEntryCollection::Concrete);

    for (PackageDatabaseEntryCollection::Iterator i(targets->begin()), i_end(targets->end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When collecting depended upon packages for '" + stringify(*i) + "':");
        DepCollector c(_imp->env, *i);
        VersionMetadata::ConstPointer metadata(_imp->env->package_database()->fetch_repository(
                    i->repository)->version_metadata(i->name, i->version));
        metadata->deps.build_depend()->accept(&c);
        metadata->deps.run_depend()->accept(&c);
        metadata->deps.post_depend()->accept(&c);
        result->insert(c.matches->begin(), c.matches->end());
    }

    return result;
}

void
UninstallList::add_unused_dependencies()
{
    Context context("When adding unused dependencies:");

    bool added(true);
    PackageDatabaseEntryCollection::ConstPointer everything(collect_all_installed());
    while (added)
    {
        added = false;

        /* find packages that're depped upon by anything in our uninstall list */
        PackageDatabaseEntryCollection::Pointer uninstall_list_targets(
                new PackageDatabaseEntryCollection::Concrete);
        for (std::list<UninstallListEntry>::const_iterator i(_imp->uninstall_list.begin()),
                i_end(_imp->uninstall_list.end()) ; i != i_end ; ++i)
            uninstall_list_targets->insert(i->package);

        Log::get_instance()->message(ll_debug, lc_context, "uninstall_list_targets is '"
                + join(uninstall_list_targets->begin(), uninstall_list_targets->end(), " ") + "'");

        PackageDatabaseEntryCollection::ConstPointer depped_upon_list(
                collect_depped_upon(uninstall_list_targets));

        Log::get_instance()->message(ll_debug, lc_context, "depped_upon_list is '"
                + join(depped_upon_list->begin(), depped_upon_list->end(), " ") + "'");

        /* find packages that're depped upon by anything not in our uninstall list */
        PackageDatabaseEntryCollection::Pointer everything_except_uninstall_list_targets(
                new PackageDatabaseEntryCollection::Concrete);
        std::set_difference(everything->begin(), everything->end(),
                uninstall_list_targets->begin(), uninstall_list_targets->end(),
                everything_except_uninstall_list_targets->inserter());

        Log::get_instance()->message(ll_debug, lc_context, "everything_except_uninstall_list_targets is '"
                + join(everything_except_uninstall_list_targets->begin(),
                    everything_except_uninstall_list_targets->end(), " ") + "'");

        PackageDatabaseEntryCollection::ConstPointer depped_upon_not_list(
                collect_depped_upon(everything_except_uninstall_list_targets));

        /* find unused dependencies */
        PackageDatabaseEntryCollection::Pointer unused_dependencies(
                new PackageDatabaseEntryCollection::Concrete);
        std::set_difference(depped_upon_list->begin(), depped_upon_list->end(),
                depped_upon_not_list->begin(), depped_upon_not_list->end(), unused_dependencies->inserter());

        Log::get_instance()->message(ll_debug, lc_context, "unused_dependencies is '"
                + join(unused_dependencies->begin(), unused_dependencies->end(), " ") + "'");

        /* if any of them aren't already on the list, add them and recurse */
        for (PackageDatabaseEntryCollection::Iterator i(unused_dependencies->begin()),
                i_end(unused_dependencies->end()) ; i != i_end ; ++i)
        {
            if (_imp->uninstall_list.end() != std::find_if(_imp->uninstall_list.begin(),
                        _imp->uninstall_list.end(), MatchUninstallListEntry(*i)))
                continue;

            add_package(*i);
            added = true;
        }
    }
}

void
UninstallList::add_dependencies(const PackageDatabaseEntry & e)
{
    Context context("When adding things that depend upon '" + stringify(e) + "':");

    PackageDatabaseEntryCollection::ConstPointer everything(collect_all_installed());
    for (PackageDatabaseEntryCollection::Iterator i(everything->begin()),
            i_end(everything->end()) ; i != i_end ; ++i)
    {
        Context local_context("When seeing whether '" + stringify(*i) + "' has a dep:");

        DepCollector c(_imp->env, *i);
        VersionMetadata::ConstPointer metadata(_imp->env->package_database()->fetch_repository(
                    i->repository)->version_metadata(i->name, i->version));
        metadata->deps.build_depend()->accept(&c);
        metadata->deps.run_depend()->accept(&c);
        metadata->deps.post_depend()->accept(&c);

        if (c.matches->end() == c.matches->find(e))
            continue;

        add(*i);
    }
}

