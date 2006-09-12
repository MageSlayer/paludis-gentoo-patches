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

#include "adjutrix_environment.hh"
#include <paludis/util/collection_concrete.hh>
#include <paludis/repositories/portage/portage_repository.hh>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<AdjutrixEnvironment> :
        InternalCounted<Implementation<AdjutrixEnvironment> >
    {
        const FSEntry top_level_dir;
        PackageDatabase::Pointer db;
        KeywordName accepted_keyword;

        Implementation(Environment * const env, const FSEntry & d) :
            top_level_dir(d),
            db(new PackageDatabase(env)),
            accepted_keyword("fnord")
        {
        }
    };
}

AdjutrixEnvironment::AdjutrixEnvironment(const FSEntry & dir) :
    PrivateImplementationPattern<AdjutrixEnvironment>(
            new Implementation<AdjutrixEnvironment>(this, dir)),
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

AdjutrixEnvironment::~AdjutrixEnvironment()
{
}

std::string
AdjutrixEnvironment::paludis_command() const
{
    return "false";
}

FSEntry
AdjutrixEnvironment::main_repository_dir() const
{
    return _imp->top_level_dir;
}

void
AdjutrixEnvironment::set_profile(const FSEntry & location)
{
    PackageDatabase::Pointer db(new PackageDatabase(this));

    AssociativeCollection<std::string, std::string>::Pointer keys(
            new AssociativeCollection<std::string, std::string>::Concrete);

    keys->insert("format", "portage");
    keys->insert("location", stringify(_imp->top_level_dir));
    keys->insert("profiles", stringify(location));

    PortageRepository::Pointer p(RepositoryMaker::get_instance()->find_maker("portage")(this,
                _imp->db.raw_pointer(), keys));
    _imp->accepted_keyword = KeywordName(p->profile_variable("ARCH"));
    db->add_repository(RepositoryMaker::get_instance()->find_maker("virtuals")(this,
                db.raw_pointer(), AssociativeCollection<std::string, std::string>::Pointer(0)));
    db->add_repository(p);

    change_package_database(db);
}

bool
AdjutrixEnvironment::accept_keyword(const KeywordName & k, const PackageDatabaseEntry * const) const
{
    return (k.data() == "*") || (k == _imp->accepted_keyword);
}

