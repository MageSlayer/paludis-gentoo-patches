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
#include <paludis/repositories/portage/portage_repository.hh>
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
        bool accept_unstable;
        PackageDatabase::Pointer db;
        std::set<KeywordName> accepted_keywords;

        Implementation(Environment * const env, const NoConfigEnvironmentParams & params) :
            top_level_dir(params.repository_dir),
            accept_unstable(params.accept_unstable),
            db(new PackageDatabase(env))
        {
        }
    };
}

NoConfigEnvironment::NoConfigEnvironment(const NoConfigEnvironmentParams & params) :
    PrivateImplementationPattern<NoConfigEnvironment>(
            new Implementation<NoConfigEnvironment>(this, params)),
    Environment(_imp->db)
{
    AssociativeCollection<std::string, std::string>::Pointer keys(
            new AssociativeCollection<std::string, std::string>::Concrete);

    keys->insert("format", "portage");
    keys->insert("location", stringify(_imp->top_level_dir));
    keys->insert("profiles", stringify(_imp->top_level_dir / "profiles" / "base"));

    _imp->db->add_repository(
            RepositoryMaker::get_instance()->find_maker("portage")(this,
                _imp->db.raw_pointer(), keys));

    _imp->db->add_repository(RepositoryMaker::get_instance()->find_maker("virtuals")(this,
                _imp->db.raw_pointer(), AssociativeCollection<std::string, std::string>::Pointer(0)));
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
NoConfigEnvironment::set_profile(const FSEntry & location)
{
    PackageDatabase::Pointer db(new PackageDatabase(this));

    AssociativeCollection<std::string, std::string>::Pointer keys(
            new AssociativeCollection<std::string, std::string>::Concrete);

    keys->insert("format", "portage");
    keys->insert("location", stringify(_imp->top_level_dir));
    keys->insert("profiles", stringify(location));

    PortageRepository::Pointer p(RepositoryMaker::get_instance()->find_maker("portage")(this,
                _imp->db.raw_pointer(), keys));
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
    return _imp->accepted_keywords.end() != _imp->accepted_keywords.find(k);
}

