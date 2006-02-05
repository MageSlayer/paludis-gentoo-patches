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

#include "default_environment.hh"
#include "package_database.hh"
#include "repository.hh"
#include "default_config.hh"
#include "stringify.hh"
#include "match_package.hh"
#include <list>
#include <vector>

using namespace paludis;

DefaultEnvironment::DefaultEnvironment() :
    Environment(PackageDatabase::Pointer(new PackageDatabase),
            PackageDatabase::Pointer(new PackageDatabase))
{
    Context context("When loading default environment:");

    for (DefaultConfig::RepositoryIterator r(DefaultConfig::get_instance()->begin_repositories()),
            r_end(DefaultConfig::get_instance()->end_repositories()) ; r != r_end ; ++r)
        package_database()->add_repository(
                RepositoryMaker::get_instance()->find_maker(r->get<rce_format>())(
                    package_database().raw_pointer(), r->get<rce_keys>()));

    /// \bug vdb
}

DefaultEnvironment::~DefaultEnvironment()
{
}

bool
DefaultEnvironment::query_use(const UseFlagName & f, const PackageDatabaseEntry * e) const
{
    if (e)
    {
        UseFlagState s(use_unspecified);

        for (DefaultConfig::UseConfigIterator
                u(DefaultConfig::get_instance()->begin_use_config(e->get<pde_name>())),
                u_end(DefaultConfig::get_instance()->end_use_config(e->get<pde_name>())) ;
                u != u_end ; ++u)
        {
            if (f != u->get<uce_flag_name>())
                continue;

            if (! match_package(package_database(), *u->get<uce_dep_atom>(), *e))
                continue;

            switch (u->get<uce_flag_state>())
            {
                case use_enabled:
                    s = use_enabled;
                    continue;

                case use_disabled:
                    s = use_disabled;
                    continue;

                case use_unspecified:
                    continue;
            }

            throw InternalError(PALUDIS_HERE, "Bad state");
        }

        do
        {
            switch (s)
            {
                case use_enabled:
                    return true;

                case use_disabled:
                    return false;

                case use_unspecified:
                    continue;
            }
            throw InternalError(PALUDIS_HERE, "Bad state");
        } while (false);
    }

    do
    {
        UseFlagState state(use_unspecified);

        for (DefaultConfig::DefaultUseIterator
                u(DefaultConfig::get_instance()->begin_default_use()),
                u_end(DefaultConfig::get_instance()->end_default_use()) ;
                u != u_end ; ++u)
            if (f == u->first)
                state = u->second;

        switch (state)
        {
            case use_enabled:
                return true;

            case use_disabled:
                return false;

            case use_unspecified:
                continue;
        }

        throw InternalError(PALUDIS_HERE, "bad state " + stringify(state));
    } while (false);

    switch (e ? package_database()->fetch_repository(e->get<pde_repository>())->query_use(f) :
            package_database()->fetch_repository(package_database()->favourite_repository())->query_use(f))
    {
        case use_disabled:
        case use_unspecified:
            return false;

        case use_enabled:
            return true;
    }

    throw InternalError(PALUDIS_HERE, "bad state");
}

bool
DefaultEnvironment::accept_keyword(const KeywordName & keyword, const PackageDatabaseEntry * const d) const
{
    if (keyword == KeywordName("*"))
        return true;

    Context context("When checking accept_keyword of '" + stringify(keyword) +
            (d ? "' for " + stringify(*d) : stringify("'")) + ":");

    bool result(false);

    if (d)
        for (DefaultConfig::PackageKeywordsIterator
                k(DefaultConfig::get_instance()->begin_package_keywords(d->get<pde_name>())),
                k_end(DefaultConfig::get_instance()->end_package_keywords(d->get<pde_name>())) ;
                k != k_end ; ++k)
        {
            if (! match_package(package_database(), k->first, d))
                continue;

            result |= k->second == keyword;
        }

    result |= DefaultConfig::get_instance()->end_default_keywords() !=
        std::find(DefaultConfig::get_instance()->begin_default_keywords(),
                DefaultConfig::get_instance()->end_default_keywords(),
                keyword);

    return result;
}

bool
DefaultEnvironment::query_user_masks(const PackageDatabaseEntry & d) const
{
    for (DefaultConfig::UserMasksIterator
            k(DefaultConfig::get_instance()->begin_user_masks(d.get<pde_name>())),
            k_end(DefaultConfig::get_instance()->end_user_masks(d.get<pde_name>())) ;
            k != k_end ; ++k)
    {
        if (! match_package(package_database(), *k, d))
            continue;

        return true;
    }

    return false;
}

bool
DefaultEnvironment::query_user_unmasks(const PackageDatabaseEntry & d) const
{
    for (DefaultConfig::UserMasksIterator
            k(DefaultConfig::get_instance()->begin_user_unmasks(d.get<pde_name>())),
            k_end(DefaultConfig::get_instance()->end_user_unmasks(d.get<pde_name>())) ;
            k != k_end ; ++k)
    {
        if (! match_package(package_database(), *k, d))
            continue;

        return true;
    }

    return false;
}

