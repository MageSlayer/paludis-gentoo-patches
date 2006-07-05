/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/package_database.hh>
#include <paludis/dep_atom.hh>
#include <paludis/environment.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/tokeniser.hh>

/** \file
 * Implementation of Environment.
 *
 * \ingroup grpenvironment
 */

using namespace paludis;

Environment::Environment(PackageDatabase::Pointer d) :
    _package_database(d),
    _has_provide_map(false)
{
}

Environment::~Environment()
{
}

namespace
{
    /**
     * Check whether licences for a package are accepted.
     */
    struct LicenceChecker :
        DepAtomVisitorTypes::ConstVisitor
    {
        /// Are all necessary licences ok?
        bool ok;

        /// Our environment.
        const Environment * const env;

        /// Our package database.
        const PackageDatabaseEntry * const db_entry;

        /// Constructor
        LicenceChecker(const Environment * const e, const PackageDatabaseEntry * const d) :
            ok(true),
            env(e),
            db_entry(d)
        {
        }

        ///\name Visit methods
        ///{
        void visit(const AllDepAtom * atom)
        {
            std::for_each(atom->begin(), atom->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * atom)
        {
            bool local_ok(false);

            if (atom->begin() == atom->end())
                local_ok = true;
            else
            {
                for (CompositeDepAtom::Iterator i(atom->begin()), i_end(atom->end()) ;
                        i != i_end ; ++i)
                {
                    Save<bool> save_ok(&ok, true);
                    (*i)->accept(this);
                    local_ok |= ok;
                }
            }

            ok &= local_ok;
        }

        void visit(const UseDepAtom * atom)
        {
            if (env->query_use(atom->flag(), db_entry))
                std::for_each(atom->begin(), atom->end(), accept_visitor(this));
        }

        void visit(const PlainTextDepAtom * atom)
        {
            if (! env->accept_license(atom->text(), db_entry))
                ok = false;
        }

        void visit(const PackageDepAtom *) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Encountered PackageDepAtom in licence?");
        }

        void visit(const BlockDepAtom *)  PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Encountered BlockDepAtom in licence?");
        }
        ///}
    };
}

MaskReasons
Environment::mask_reasons(const PackageDatabaseEntry & e) const
{
    Context context("When checking mask reasons for '" + stringify(e) + "'");

    MaskReasons result;
    VersionMetadata::ConstPointer metadata(package_database()->fetch_repository(
                e.get<pde_repository>())->version_metadata(e.get<pde_name>(), e.get<pde_version>()));

    if (metadata->get<vm_eapi>() != "0" && metadata->get<vm_eapi>() != ""
            && metadata->get<vm_eapi>() != "paludis-1")
        result.set(mr_eapi);
    else
    {
        if (metadata->get_ebuild_interface())
        {
            std::set<KeywordName> keywords;
            WhitespaceTokeniser::get_instance()->tokenise(
                    metadata->get_ebuild_interface()->get<evm_keywords>(),
                    create_inserter<KeywordName>(std::inserter(keywords, keywords.end())));

            result.set(mr_keyword);
            for (std::set<KeywordName>::const_iterator i(keywords.begin()),
                    i_end(keywords.end()) ; i != i_end ; ++i)
                if (accept_keyword(*i, &e))
                {
                    result.reset(mr_keyword);
                    break;
                }

            if (! metadata->get_ebuild_interface()->get<evm_virtual>().empty())
            {
                QualifiedPackageName n(metadata->get_ebuild_interface()->get<evm_virtual>());

                PackageDatabaseEntry ee(n, e.get<pde_version>(), e.get<pde_repository>());
                std::set<KeywordName> keywords;
                WhitespaceTokeniser::get_instance()->tokenise(
                        metadata->get_ebuild_interface()->get<evm_keywords>(),
                        create_inserter<KeywordName>(std::inserter(keywords, keywords.end())));

                for (std::set<KeywordName>::const_iterator i(keywords.begin()),
                        i_end(keywords.end()) ; i != i_end ; ++i)
                    if (accept_keyword(*i, &ee))
                    {
                        result.reset(mr_keyword);
                        break;
                    }
            }
        }

        LicenceChecker lc(this, &e);
        metadata->license()->accept(&lc);
        if (! lc.ok)
            result.set(mr_license);

        if (! query_user_unmasks(e))
        {
            if (query_user_masks(e))
                result.set(mr_user_mask);

            const Repository * const repo(package_database()->fetch_repository(
                        e.get<pde_repository>()).raw_pointer());

            if (repo->get_interface<repo_mask>())
            {
                if (repo->get_interface<repo_mask>()->query_profile_masks(e.get<pde_name>(),
                            e.get<pde_version>()))
                    result.set(mr_profile_mask);

                if (repo->get_interface<repo_mask>()->query_repository_masks(e.get<pde_name>(),
                            e.get<pde_version>()))
                    result.set(mr_repository_mask);

                if (metadata->get_ebuild_interface())
                    if (! metadata->get_ebuild_interface()->get<evm_virtual>().empty())
                    {
                        QualifiedPackageName n(metadata->get_ebuild_interface()->get<evm_virtual>());

                        if (repo->get_interface<repo_mask>()->query_profile_masks(n,
                                    e.get<pde_version>()))
                            result.set(mr_profile_mask);

                        if (repo->get_interface<repo_mask>()->query_repository_masks(n,
                                    e.get<pde_version>()))
                            result.set(mr_repository_mask);
                    }
            }
        }
    }

    return result;
}

Environment::ProvideMapIterator
Environment::begin_provide_map() const
{
    if (! _has_provide_map)
    {
        Context context("When scanning for PROVIDEs:");

        for (PackageDatabase::RepositoryIterator r(package_database()->begin_repositories()),
                r_end(package_database()->end_repositories()) ; r != r_end ; ++r)
        {
            if (! (*r)->get_interface<repo_installed>())
                continue;

            std::copy((*r)->begin_provide_map(), (*r)->end_provide_map(),
                    std::inserter(_provide_map, _provide_map.begin()));
        }

        _has_provide_map = true;
    }

    return _provide_map.begin();
}

Environment::ProvideMapIterator
Environment::end_provide_map() const
{
    return _provide_map.end();
}

DepAtom::Pointer
Environment::package_set(const std::string & s, const PackageSetOptions & o) const
{
    if (s == "everything" || s == "system" || s == "world" || s == "security")
    {
        AllDepAtom::Pointer result(new AllDepAtom);

        for (PackageDatabase::RepositoryIterator r(package_database()->begin_repositories()),
                r_end(package_database()->end_repositories()) ;
                r != r_end ; ++r)
        {
            if (! (*r)->get_interface<repo_sets>())
                continue;

            DepAtom::Pointer add((*r)->get_interface<repo_sets>()->package_set(s, o));
            if (0 != add)
                result->add_child(add);

            if ("system" != s)
            {
                add = (*r)->get_interface<repo_sets>()->package_set("system");
                if (0 != add)
                    result->add_child(add);
            }
        }

        return result;
    }
    else
    {
        DepAtom::Pointer rr(local_package_set(s));
        if (0 != rr)
            return rr;

        for (PackageDatabase::RepositoryIterator r(package_database()->begin_repositories()),
                r_end(package_database()->end_repositories()) ;
                r != r_end ; ++r)
        {
            if (! (*r)->get_interface<repo_sets>())
                continue;

            DepAtom::Pointer result((*r)->get_interface<repo_sets>()->package_set(s));
            if (0 != result)
                return result;
        }

        return DepAtom::Pointer(0);
    }
}

namespace
{
    /**
     * Find package targets that are appropriate for adding to or removing
     * from the world file.
     */
    struct WorldTargetFinder :
        DepAtomVisitorTypes::ConstVisitor
    {
        /// Matches
        std::list<const PackageDepAtom *> items;

        ///\name Visit methods
        ///{
        void visit(const AllDepAtom * a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom *)
        {
        }

        void visit(const UseDepAtom *)
        {
        }

        void visit(const PlainTextDepAtom *)
        {
        }

        void visit(const PackageDepAtom * a)
        {
            if (! (a->slot_ptr() || a->version_spec_ptr()))
                items.push_back(a);
        }

        void visit(const BlockDepAtom *)
        {
        }
        ///}

    };
}

void
Environment::add_appropriate_to_world(DepAtom::ConstPointer a) const
{
    WorldTargetFinder w;
    a->accept(&w);
    for (std::list<const PackageDepAtom *>::const_iterator i(w.items.begin()),
            i_end(w.items.end()) ; i != i_end ; ++i)
    {
        if (! package_database()->query(**i, is_installed_only)->empty())
            Log::get_instance()->message(ll_debug, lc_no_context,
                    "Not adding '" + stringify(**i) +
                    "' to world, because it is already installed");
        else
        {
            for (PackageDatabase::RepositoryIterator r(package_database()->begin_repositories()),
                    r_end(package_database()->end_repositories()) ;
                    r != r_end ; ++r)
                if ((*r)->get_interface<repo_world>())
                    (*r)->get_interface<repo_world>()->add_to_world((*i)->package());
        }
    }
}

void
Environment::remove_appropriate_from_world(DepAtom::ConstPointer a) const
{
    WorldTargetFinder w;
    a->accept(&w);
    for (std::list<const PackageDepAtom *>::const_iterator i(w.items.begin()),
            i_end(w.items.end()) ; i != i_end ; ++i)
    {
        for (PackageDatabase::RepositoryIterator r(package_database()->begin_repositories()),
                r_end(package_database()->end_repositories()) ;
                r != r_end ; ++r)
            if ((*r)->get_interface<repo_world>())
                (*r)->get_interface<repo_world>()->remove_from_world((*i)->package());
    }
}

Hook::Hook(const std::string & n) :
    _name(n)
{
}

Hook
Hook::operator() (const std::string & k, const std::string & v) const
{
    Hook result(*this);
    result._extra_env.insert(std::make_pair(k, v));
    return result;
}

