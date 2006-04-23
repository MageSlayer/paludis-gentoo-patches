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

#include <list>
#include <paludis/default_config.hh>
#include <paludis/default_environment.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
#include <vector>

using namespace paludis;

DefaultEnvironment::DefaultEnvironment() :
    Environment(PackageDatabase::Pointer(new PackageDatabase(this)))
{
    Context context("When loading default environment:");

    for (DefaultConfig::RepositoryIterator r(DefaultConfig::get_instance()->begin_repositories()),
            r_end(DefaultConfig::get_instance()->end_repositories()) ; r != r_end ; ++r)
        package_database()->add_repository(
                RepositoryMaker::get_instance()->find_maker(r->get<rce_format>())(
                    this, package_database().raw_pointer(), r->get<rce_keys>()));
}

DefaultEnvironment::~DefaultEnvironment()
{
}

bool
DefaultEnvironment::query_use(const UseFlagName & f, const PackageDatabaseEntry * e) const
{
    /* first check package database use masks... */
    if (e ? package_database()->fetch_repository(e->get<pde_repository>())->query_use_mask(f, e) :
            package_database()->fetch_repository(
                package_database()->favourite_repository())->query_use_mask(f, e))
        return false;

    /* check use: per package user config */
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

            if (! match_package(this, *u->get<uce_dep_atom>(), *e))
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

    /* check use: general user config */
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

    /* check use: package database config */
    switch (e ? package_database()->fetch_repository(e->get<pde_repository>())->query_use(f, e) :
            package_database()->fetch_repository(package_database()->favourite_repository())->query_use(f, e))
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
            if (! match_package(this, k->first, d))
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
DefaultEnvironment::accept_license(const std::string & license, const PackageDatabaseEntry * const d) const
{
    if (license == "*")
        return true;

    Context context("When checking license of '" + license +
            (d ? "' for " + stringify(*d) : stringify("'")) + ":");

    bool result(false);

    if (d)
        for (DefaultConfig::PackageLicensesIterator
                k(DefaultConfig::get_instance()->begin_package_licenses(d->get<pde_name>())),
                k_end(DefaultConfig::get_instance()->end_package_licenses(d->get<pde_name>())) ;
                k != k_end ; ++k)
        {
            if (! match_package(this, k->first, d))
                continue;

            result |= k->second == license;
            result |= k->second == "*";
        }

    result |= DefaultConfig::get_instance()->end_default_licenses() !=
        std::find(DefaultConfig::get_instance()->begin_default_licenses(),
                DefaultConfig::get_instance()->end_default_licenses(),
                license);

    result |= DefaultConfig::get_instance()->end_default_licenses() !=
        std::find(DefaultConfig::get_instance()->begin_default_licenses(),
                DefaultConfig::get_instance()->end_default_licenses(),
                "*");

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
        if (! match_package(this, *k, d))
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
        if (! match_package(this, *k, d))
            continue;

        return true;
    }

    return false;
}

std::string
DefaultEnvironment::bashrc_files() const
{
    return DefaultConfig::get_instance()->bashrc_files();
}

std::string
DefaultEnvironment::paludis_command() const
{
    return DefaultConfig::get_instance()->paludis_command();
}

UseFlagNameCollection::Pointer
DefaultEnvironment::query_enabled_use_matching(const std::string & prefix,
        const PackageDatabaseEntry * e) const
{
    UseFlagNameCollection::Pointer result(new UseFlagNameCollection);

    for (DefaultConfig::DefaultUseIterator
            u(DefaultConfig::get_instance()->begin_default_use()),
            u_end(DefaultConfig::get_instance()->end_default_use()) ;
            u != u_end ; ++u)
    {
        if (0 != u->first.data().compare(0, prefix.length(), prefix))
            continue;

        switch (u->second)
        {
            case use_enabled:
                result->insert(u->first);
                break;

            case use_disabled:
                result->erase(u->first);
                break;

            case use_unspecified:
                break;
        }
    }

    if (e)
    {
        for (DefaultConfig::UseConfigIterator
                u(DefaultConfig::get_instance()->begin_use_config(e->get<pde_name>())),
                u_end(DefaultConfig::get_instance()->end_use_config(e->get<pde_name>())) ;
                u != u_end ; ++u)
        {
            if (0 != u->get<uce_flag_name>().data().compare(0, prefix.length(), prefix))
                continue;

            if (! match_package(this, *u->get<uce_dep_atom>(), *e))
                continue;

            switch (u->get<uce_flag_state>())
            {
                case use_enabled:
                    result->insert(u->get<uce_flag_name>());
                    break;

                case use_disabled:
                    result->erase(u->get<uce_flag_name>());
                    break;

                case use_unspecified:
                    break;
            }
        }
    }

    return result;
}

