/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/collection_concrete.hh>

#include <set>
#include <list>

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
                e.repository)->version_metadata(e.name, e.version));

    if (metadata->eapi != "0" && metadata->eapi != ""
            && metadata->eapi != "paludis-1" && metadata->eapi != "CRAN-0")
        result.set(mr_eapi);
    else
    {
        if (metadata->get_virtual_interface())
        {
            result |= mask_reasons(metadata->get_virtual_interface()->virtual_for);
            if (result.any())
                result.set(mr_by_association);
        }

        if (metadata->get_ebuild_interface())
        {
            std::set<KeywordName> keywords;
            WhitespaceTokeniser::get_instance()->tokenise(
                    metadata->get_ebuild_interface()->keywords,
                    create_inserter<KeywordName>(std::inserter(keywords, keywords.end())));

            result.set(mr_keyword);
            for (std::set<KeywordName>::const_iterator i(keywords.begin()),
                    i_end(keywords.end()) ; i != i_end ; ++i)
                if (accept_keyword(*i, &e))
                {
                    result.reset(mr_keyword);
                    break;
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
                        e.repository).raw_pointer());

            if (repo->mask_interface)
            {
                if (repo->mask_interface->query_profile_masks(e.name,
                            e.version))
                    result.set(mr_profile_mask);

                if (repo->mask_interface->query_repository_masks(e.name,
                            e.version))
                    result.set(mr_repository_mask);
            }
        }
    }

    return result;
}

DepAtom::Pointer
Environment::package_set(const std::string & s, const PackageSetOptions & o) const
{
    /* favour local sets first */
    CompositeDepAtom::Pointer result(local_package_set(s));
    if (0 != result)
        return result;

    /* these sets always exist, even if empty */
    if (s == "everything" || s == "system" || s == "world" || s == "security")
        result.assign(new AllDepAtom);

    for (PackageDatabase::RepositoryIterator r(package_database()->begin_repositories()),
            r_end(package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (! (*r)->sets_interface)
            continue;

        DepAtom::Pointer add((*r)->sets_interface->package_set(s, o));
        if (0 != add)
        {
            if (! result)
                result.assign(new AllDepAtom);
            result->add_child(add);
        }

        if ("everything" == s || "world" == s)
        {
            add = (*r)->sets_interface->package_set("system");
            if (0 != add)
                result->add_child(add);
        }
    }

    return result;
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

        /// Callback object pointer, may be 0.
        Environment::WorldCallbacks * const w;

        /// Are we inside a || ( ) group?
        bool inside_any;

        /// Are we inside a use? ( ) group?
        bool inside_use;

        /// Constructor.
        WorldTargetFinder(Environment::WorldCallbacks * const ww) :
            w(ww),
            inside_any(false),
            inside_use(false)
        {
        }

        ///\name Visit methods
        ///{
        void visit(const AllDepAtom * a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * a)
        {
            Save<bool> save_inside_any(&inside_any, true);
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const UseDepAtom * a)
        {
            Save<bool> save_inside_use(&inside_use, true);
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const PlainTextDepAtom *)
        {
        }

        void visit(const PackageDepAtom * a)
        {
            if (inside_any)
            {
                if (w)
                    w->skip_callback(a, "inside || ( ) block");
            }
            else if (inside_use)
            {
                if (w)
                    w->skip_callback(a, "inside use? ( ) block");
            }
            else if (a->slot_ptr())
            {
                if (w)
                    w->skip_callback(a, ":slot restrictions");
            }
            else if (a->version_spec_ptr())
            {
                if (w)
                    w->skip_callback(a, "version restrictions");
            }
            else
            {
                items.push_back(a);
                if (w)
                    w->add_callback(a);
            }
        }

        void visit(const BlockDepAtom *)
        {
        }
        ///}

    };
}

void
Environment::add_appropriate_to_world(DepAtom::ConstPointer a,
        Environment::WorldCallbacks * const ww) const
{
    WorldTargetFinder w(ww);
    a->accept(&w);
    for (std::list<const PackageDepAtom *>::const_iterator i(w.items.begin()),
            i_end(w.items.end()) ; i != i_end ; ++i)
    {
        for (PackageDatabase::RepositoryIterator r(package_database()->begin_repositories()),
                r_end(package_database()->end_repositories()) ;
                r != r_end ; ++r)
            if ((*r)->world_interface)
                (*r)->world_interface->add_to_world((*i)->package());
    }
}

void
Environment::remove_appropriate_from_world(DepAtom::ConstPointer a,
        Environment::WorldCallbacks * const ww) const
{
    WorldTargetFinder w(ww);
    a->accept(&w);
    for (std::list<const PackageDepAtom *>::const_iterator i(w.items.begin()),
            i_end(w.items.end()) ; i != i_end ; ++i)
    {
        for (PackageDatabase::RepositoryIterator r(package_database()->begin_repositories()),
                r_end(package_database()->end_repositories()) ;
                r != r_end ; ++r)
            if ((*r)->world_interface)
                (*r)->world_interface->remove_from_world((*i)->package());

        ww->remove_callback(*i);
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

bool
Environment::query_use(const UseFlagName & f, const PackageDatabaseEntry * e) const
{
    /* first check package database use masks... */
    const Repository * const repo((e ?
                package_database()->fetch_repository(e->repository).raw_pointer() :
                0));

    if (repo && repo->use_interface)
    {
        if (repo->use_interface->query_use_mask(f, e))
            return false;
        if (repo->use_interface->query_use_force(f, e))
            return true;
    }

    /* check use: package database config */
    if (repo && repo->use_interface)
    {
        switch (repo->use_interface->query_use(f, e))
        {
            case use_disabled:
            case use_unspecified:
                return false;

            case use_enabled:
                return true;
        }

        throw InternalError(PALUDIS_HERE, "bad state");
    }
    else
    {
        return false;
    }
}

bool
Environment::accept_keyword(const KeywordName & keyword, const PackageDatabaseEntry * const) const
{
    if (keyword == KeywordName("*"))
        return true;

    return false;
}

bool
Environment::accept_license(const std::string &, const PackageDatabaseEntry * const) const
{
    return true;
}

bool
Environment::query_user_masks(const PackageDatabaseEntry &) const
{
    return false;
}

bool
Environment::query_user_unmasks(const PackageDatabaseEntry &) const
{
    return false;
}

std::string
Environment::bashrc_files() const
{
    return "";
}


std::string
Environment::hook_dirs() const
{
    return "";
}

namespace
{
    static const std::multimap<std::string, std::string> environment_mirrors;
}

Environment::MirrorIterator
Environment::begin_mirrors(const std::string &) const
{
    return MirrorIterator(environment_mirrors.begin());
}

Environment::MirrorIterator
Environment::end_mirrors(const std::string &) const
{
    return MirrorIterator(environment_mirrors.end());
}

void
Environment::perform_hook(const Hook &) const
{
}

UseFlagNameCollection::ConstPointer
Environment::known_use_expand_names(const UseFlagName &, const PackageDatabaseEntry *) const
{
    return UseFlagNameCollection::ConstPointer(new UseFlagNameCollection::Concrete);
}

