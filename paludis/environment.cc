/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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
#include <paludis/dep_parser.hh>
#include <paludis/qa/environment.hh>

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
    struct LicenceChecker :
        DepAtomVisitorTypes::ConstVisitor
    {
        bool ok;
        const Environment * const env;
        const PackageDatabaseEntry * const db_entry;

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
            std::for_each(atom->begin(), atom->end(), accept_visitor(this));
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
            throw InternalError(PALUDIS_HERE, "todo: encountered PackageDepAtom in licence?"); /// \bug todo
        }

        void visit(const BlockDepAtom *)  PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "todo: encountered BlockDepAtom in licence?"); /// \bug todo
        }
        ///}
    };
}

MaskReasons
Environment::mask_reasons(const PackageDatabaseEntry & e) const
{
    Context context("When checking mask reasons for '" + stringify(e) + "'");

    MaskReasons result;
    VersionMetadata::ConstPointer metadata(package_database()->fetch_metadata(e));

    if (metadata->get(vmk_eapi) != "0" && metadata->get(vmk_eapi) != ""
            && metadata->get(vmk_eapi) != "paludis-1")
        result.set(mr_eapi);
    else
    {
        result.set(mr_keyword);
        for (VersionMetadata::KeywordIterator k(metadata->begin_keywords()),
                k_end(metadata->end_keywords()) ; k != k_end ; ++k)
            if (accept_keyword(*k, &e))
            {
                result.reset(mr_keyword);
                break;
            }

        LicenceChecker lc(this, &e);
        DepParser::parse(metadata->get(vmk_license),
                DepParserPolicy<PlainTextDepAtom, true>::get_instance())->accept(&lc);
        if (! lc.ok)
            result.set(mr_license);

        if (! metadata->get(vmk_virtual).empty())
        {
            QualifiedPackageName n(metadata->get(vmk_virtual));

            PackageDatabaseEntry ee(n, e.get<pde_version>(), e.get<pde_repository>());
            for (VersionMetadata::KeywordIterator k(metadata->begin_keywords()),
                    k_end(metadata->end_keywords()) ; k != k_end ; ++k)
                if (accept_keyword(*k, &ee))
                {
                    result.reset(mr_keyword);
                    break;
                }
        }

        if (! query_user_unmasks(e))
        {
            if (query_user_masks(e))
                result.set(mr_user_mask);

            if (package_database()->fetch_repository(e.get<pde_repository>())->query_profile_masks(e.get<pde_name>(),
                        e.get<pde_version>()))
                result.set(mr_profile_mask);

            if (package_database()->fetch_repository(e.get<pde_repository>())->query_repository_masks(e.get<pde_name>(),
                        e.get<pde_version>()))
                result.set(mr_repository_mask);

            if (! metadata->get(vmk_virtual).empty())
            {
                QualifiedPackageName n(metadata->get(vmk_virtual));

                if (package_database()->fetch_repository(e.get<pde_repository>())->query_profile_masks(n,
                            e.get<pde_version>()))
                    result.set(mr_profile_mask);

                if (package_database()->fetch_repository(e.get<pde_repository>())->query_repository_masks(n,
                            e.get<pde_version>()))
                    result.set(mr_repository_mask);
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
            if (! (*r)->installed())
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

