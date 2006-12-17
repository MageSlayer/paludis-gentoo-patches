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

#include "no_config_environment.hh"
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/repositories/portage/portage_repository.hh>
#include <paludis/config_file.hh>
#include <set>

using namespace paludis;

#include <paludis/environment/no_config/no_config_environment-sr.cc>

namespace paludis
{
    template<>
    struct Implementation<NoConfigEnvironment> :
        InternalCounted<Implementation<NoConfigEnvironment> >
    {
        const FSEntry top_level_dir;
        const FSEntry write_cache;
        bool accept_unstable;
        bool is_vdb;

        PortageRepository::Pointer portage_repo;

        Implementation(Environment * const env, const NoConfigEnvironmentParams & params);
    };

    /* This goat is for Dave Wickham */
}

namespace
{
    bool is_vdb_repository(const FSEntry & location, NoConfigEnvironmentRepositoryType type)
    {
        switch (type)
        {
            case ncer_portage:
                return false;
            case ncer_vdb:
                return true;
            case ncer_auto:
                ;
        }

        Context context("When determining repository type at '" + stringify(location) + "':");

        if (! location.is_directory())
            throw ConfigurationError("Location is not a directory");

        if ((location / "profiles").is_directory())
        {
            Log::get_instance()->message(ll_debug, lc_context, "Found profiles/, looks like Portage format");
            return false;
        }

        int outer_count(0);
        for (DirIterator d(location), d_end ; d != d_end ; ++d)
        {
            if (! d->is_directory())
                continue;

            int inner_count(0);
            for (DirIterator e(*d), e_end ; e != e_end ; ++e)
            {
                if (! e->is_directory())
                    continue;

                if ((*e / "CONTENTS").exists())
                {
                    Log::get_instance()->message(ll_debug, lc_context, "Found '" + stringify(*e) +
                            "/CONTENTS', looks like VDB format");
                    return true;
                }

                if (inner_count++ >= 5)
                    break;
            }

            if (outer_count++ >= 5)
                break;
        }

        throw ConfigurationError("Can't work out what kind of repository this is");
    }
}

Implementation<NoConfigEnvironment>::Implementation(
        Environment * const env, const NoConfigEnvironmentParams & params) :
    top_level_dir(params.repository_dir),
    write_cache(params.write_cache),
    accept_unstable(params.accept_unstable),
    is_vdb(is_vdb_repository(params.repository_dir, params.repository_type)),
    portage_repo(0)
{
    Context context("When initialising NoConfigEnvironment at '" + stringify(params.repository_dir) + "':");

    if (! is_vdb)
    {
        AssociativeCollection<std::string, std::string>::Pointer keys(
                new AssociativeCollection<std::string, std::string>::Concrete);

        keys->insert("format", "portage");
        keys->insert("location", stringify(params.repository_dir));
        keys->insert("profiles", "/var/empty");
        keys->insert("write_cache", stringify(params.write_cache));
        keys->insert("names_cache", "/var/empty");

        env->package_database()->add_repository(((portage_repo =
                        RepositoryMaker::get_instance()->find_maker("portage")(env,
                            env->package_database().raw_pointer(), keys))));
        env->package_database()->add_repository(RepositoryMaker::get_instance()->find_maker("virtuals")(env,
                    env->package_database().raw_pointer(), AssociativeCollection<std::string, std::string>::Pointer(0)));
    }
    else
    {
        Log::get_instance()->message(ll_debug, lc_context, "VDB, using vdb_db");

        AssociativeCollection<std::string, std::string>::Pointer keys(
                new AssociativeCollection<std::string, std::string>::Concrete);

        keys->insert("format", "vdb");
        keys->insert("names_cache", "/var/empty");
        keys->insert("provides_cache", "/var/empty");
        keys->insert("location", stringify(top_level_dir));

        env->package_database()->add_repository(RepositoryMaker::get_instance()->find_maker("vdb")(env,
                    env->package_database().raw_pointer(), keys));
        env->package_database()->add_repository(RepositoryMaker::get_instance()->find_maker("installed_virtuals")(env,
                    env->package_database().raw_pointer(), AssociativeCollection<std::string, std::string>::Pointer(0)));
    }
}

NoConfigEnvironment::NoConfigEnvironment(const NoConfigEnvironmentParams & params) :
    Environment(PackageDatabase::Pointer(new PackageDatabase(this))),
    PrivateImplementationPattern<NoConfigEnvironment>(
            new Implementation<NoConfigEnvironment>(this, params))
{
    if (_imp->portage_repo)
        if (_imp->portage_repo->end_profiles() != _imp->portage_repo->begin_profiles())
            _imp->portage_repo->set_profile(_imp->portage_repo->begin_profiles());
}

NoConfigEnvironment::~NoConfigEnvironment()
{
}

std::string
NoConfigEnvironment::paludis_command() const
{
    return "false";
}

FSEntry
NoConfigEnvironment::main_repository_dir() const
{
    return _imp->top_level_dir;
}

bool
NoConfigEnvironment::accept_keyword(const KeywordName & k, const PackageDatabaseEntry * const) const
{
    if (_imp->is_vdb)
        return true;

    Log::get_instance()->message(ll_debug, lc_no_context, "accept_keyword " + stringify(k) + ":");
    std::string accept_keywords(_imp->portage_repo->profile_variable("ACCEPT_KEYWORDS"));
    if (accept_keywords.empty())
    {
        std::string arch(_imp->portage_repo->profile_variable("ARCH"));
        if (stringify(k) == arch)
        {
            Log::get_instance()->message(ll_debug, lc_no_context, "accept_keyword match on arch");
            return true;
        }

        if (_imp->accept_unstable && ("~" + stringify(k) == arch))
        {
            Log::get_instance()->message(ll_debug, lc_no_context, "accept_keyword match on ~arch");
            return true;
        }

        Log::get_instance()->message(ll_debug, lc_no_context, "accept_keyword no match on arch");
    }
    else
    {
        std::list<KeywordName> accepted;
        WhitespaceTokeniser::get_instance()->tokenise(accept_keywords,
                create_inserter<KeywordName>(std::back_inserter(accepted)));

        if (accepted.end() != std::find(accepted.begin(), accepted.end(), k))
        {
            Log::get_instance()->message(ll_debug, lc_no_context, "accept_keyword match on accepted");
            return true;
        }

        if (_imp->accept_unstable && '~' == stringify(k).at(0))
        {
            if (accepted.end() != std::find(accepted.begin(), accepted.end(),
                        KeywordName(stringify(k).substr(1))))
            {
                Log::get_instance()->message(ll_debug, lc_no_context, "accept_keyword match on ~accepted");
                return true;
            }

            Log::get_instance()->message(ll_debug, lc_no_context, "accept_keyword no match on ~accepted");
        }
        else
            Log::get_instance()->message(ll_debug, lc_no_context, "accept_keyword no match on accepted");
    }

    return false;
}

void
NoConfigEnvironment::set_accept_unstable(const bool value)
{
    _imp->accept_unstable = value;
}

PortageRepository::Pointer
NoConfigEnvironment::portage_repository()
{
    return _imp->portage_repo;
}

PortageRepository::ConstPointer
NoConfigEnvironment::portage_repository() const
{
    return _imp->portage_repo;
}

