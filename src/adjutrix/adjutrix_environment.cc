/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<AdjutrixEnvironment> :
        InternalCounted<Implementation<AdjutrixEnvironment> >
    {
        const FSEntry top_level_dir;
        PackageDatabase::Pointer db;

        Implementation(Environment * const env, const FSEntry & d) :
            top_level_dir(d),
            db(new PackageDatabase(env))
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
}

AdjutrixEnvironment::~AdjutrixEnvironment()
{
}

std::string
AdjutrixEnvironment::paludis_command() const
{
    return "false";
}

