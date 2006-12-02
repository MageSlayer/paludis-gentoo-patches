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
        std::set<KeywordName> accepted_keywords;
        std::list<NoConfigEnvironmentProfilesDescLine> profiles;
        PackageDatabase::Pointer vdb_db;

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
    vdb_db(is_vdb ? new PackageDatabase(env) : 0)
{
    Context context("When initialising NoConfigEnvironment at '" + stringify(params.repository_dir) + "':");

    if (! is_vdb)
    {
        Log::get_instance()->message(ll_debug, lc_context, "Not VDB, using profiles.desc");
        try
        {
            LineConfigFile profiles_desc(params.repository_dir / "profiles" / "profiles.desc");
            for (LineConfigFile::Iterator line(profiles_desc.begin()), line_end(profiles_desc.end()) ;
                    line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

                if (tokens.size() != 3)
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Skipping invalid line '"
                            + *line + "'");
                    continue;
                }

                profiles.push_back(NoConfigEnvironmentProfilesDescLine::create()
                        .path(params.repository_dir / "profiles" / tokens.at(1))
                        .status(tokens.at(2))
                        .arch(tokens.at(0))
                        .db(PackageDatabase::Pointer(new PackageDatabase(env))));
            }
        }
        catch (const ConfigFileError & e)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Could not load profiles.desc due to exception '"
                    + e.message() + "' (" + e.what() + ")");
        }

        if (profiles.empty())
            profiles.push_back(NoConfigEnvironmentProfilesDescLine::create()
                    .path(params.repository_dir / "profiles" / "base")
                    .status("default")
                    .arch("default")
                    .db(PackageDatabase::Pointer(new PackageDatabase(env))));

        for (std::list<NoConfigEnvironmentProfilesDescLine>::iterator
                p(profiles.begin()), p_end(profiles.end()) ; p != p_end ; ++p)
        {
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);

            keys->insert("format", "portage");
            keys->insert("location", stringify(params.repository_dir));
            keys->insert("profiles", stringify(p->path));
            keys->insert("write_cache", stringify(params.write_cache));
            keys->insert("names_cache", "/var/empty");

            p->db->add_repository(RepositoryMaker::get_instance()->find_maker("portage")(env,
                        p->db.raw_pointer(), keys));
            p->db->add_repository(RepositoryMaker::get_instance()->find_maker("virtuals")(env,
                        p->db.raw_pointer(), AssociativeCollection<std::string, std::string>::Pointer(0)));
        }
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

        vdb_db->add_repository(RepositoryMaker::get_instance()->find_maker("vdb")(env,
                    vdb_db.raw_pointer(), keys));
        vdb_db->add_repository(RepositoryMaker::get_instance()->find_maker("installed_virtuals")(env,
                    vdb_db.raw_pointer(), AssociativeCollection<std::string, std::string>::Pointer(0)));
    }
}

NoConfigEnvironment::NoConfigEnvironment(const NoConfigEnvironmentParams & params) :
    PrivateImplementationPattern<NoConfigEnvironment>(
            new Implementation<NoConfigEnvironment>(this, params)),
    Environment(_imp->is_vdb ? _imp->vdb_db : _imp->profiles.begin()->db)
{
    /* do this to load accepted_keywords etc */
    if (! _imp->is_vdb)
        set_profile(ProfilesIterator(_imp->profiles.begin()));
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

void
NoConfigEnvironment::set_profile(const NoConfigEnvironment::ProfilesIterator & i)
{
    if (_imp->is_vdb)
        throw ConfigurationError("Calling NoConfigEnvironment::set_profile but no Portage repository was defined");

    change_package_database(i->db);

    _imp->accepted_keywords.clear();
    _imp->accepted_keywords.insert(KeywordName("*"));
    _imp->accepted_keywords.insert(KeywordName(i->arch));
    if (_imp->accept_unstable)
        _imp->accepted_keywords.insert(KeywordName("~" + i->arch));
}

void
NoConfigEnvironment::set_profile(const FSEntry & location)
{
    Context context("When setting NoConfigEnvironment profile to '" + stringify(location) + "':");

    if (_imp->is_vdb)
        throw ConfigurationError("Calling NoConfigEnvironment::set_profile but no Portage repository was defined");

    for (ProfilesIterator i(begin_profiles()), i_end(end_profiles()) ; i != i_end ; ++i)
        if (i->path == location)
        {
            set_profile(i);
            return;
        }

    Log::get_instance()->message(ll_warning, lc_context, "No profiles.desc entry found, faking it");

    PackageDatabase::Pointer db(new PackageDatabase(this));
    AssociativeCollection<std::string, std::string>::Pointer keys(
            new AssociativeCollection<std::string, std::string>::Concrete);

    keys->insert("format", "portage");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(_imp->top_level_dir));
    keys->insert("profiles", stringify(location));
    keys->insert("write_cache", stringify(_imp->write_cache));

    PortageRepository::Pointer p(RepositoryMaker::get_instance()->find_maker("portage")(this,
                db.raw_pointer(), keys));
    db->add_repository(RepositoryMaker::get_instance()->find_maker("virtuals")(this,
                db.raw_pointer(), AssociativeCollection<std::string, std::string>::Pointer(0)));
    db->add_repository(p);

    _imp->accepted_keywords.clear();
    _imp->accepted_keywords.insert(KeywordName("*"));
    _imp->accepted_keywords.insert(KeywordName(p->profile_variable("ARCH")));
    if (_imp->accept_unstable)
        _imp->accepted_keywords.insert(KeywordName("~" + p->profile_variable("ARCH")));

    change_package_database(db);
}

bool
NoConfigEnvironment::accept_keyword(const KeywordName & k, const PackageDatabaseEntry * const) const
{
    return _imp->is_vdb || (_imp->accepted_keywords.end() != _imp->accepted_keywords.find(k));
}

NoConfigEnvironment::ProfilesIterator
NoConfigEnvironment::begin_profiles() const
{
    return ProfilesIterator(_imp->profiles.begin());
}

NoConfigEnvironment::ProfilesIterator
NoConfigEnvironment::end_profiles() const
{
    return ProfilesIterator(_imp->profiles.end());
}

