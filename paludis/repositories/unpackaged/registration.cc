/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/util/attributes.hh>
#include <paludis/util/set.hh>
#include <paludis/repository_factory.hh>
#include <paludis/repositories/unpackaged/installed_repository.hh>
#include <paludis/repositories/unpackaged/unpackaged_repository.hh>

using namespace paludis;

extern "C" void paludis_initialise_repository_so(RepositoryFactory * const factory) PALUDIS_VISIBLE;

void paludis_initialise_repository_so(RepositoryFactory * const factory)
{
    std::tr1::shared_ptr<Set<std::string> > unpackaged_formats(new Set<std::string>);
    unpackaged_formats->insert("unpackaged");

    factory->add_repository_format(
            unpackaged_formats,
            &UnpackagedRepository::repository_factory_name,
            &UnpackagedRepository::repository_factory_create,
            &UnpackagedRepository::repository_factory_dependencies
            );

    std::tr1::shared_ptr<Set<std::string> > installed_unpackaged_formats(new Set<std::string>);
    installed_unpackaged_formats->insert("installed_unpackaged");
    installed_unpackaged_formats->insert("installed-unpackaged");

    factory->add_repository_format(
            installed_unpackaged_formats,
            &InstalledUnpackagedRepository::repository_factory_name,
            &InstalledUnpackagedRepository::repository_factory_create,
            &InstalledUnpackagedRepository::repository_factory_dependencies
            );
}

