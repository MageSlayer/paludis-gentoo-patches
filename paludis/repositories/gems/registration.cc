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

#include <paludis/repository_factory.hh>
#include <paludis/repositories/gems/gems_repository.hh>
#include <paludis/repositories/gems/installed_gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/exceptions.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>

using namespace paludis;

extern "C" void paludis_initialise_repository_so(RepositoryFactory * const factory) PALUDIS_VISIBLE;

void paludis_initialise_repository_so(RepositoryFactory * const factory)
{
    std::tr1::shared_ptr<Set<std::string> > gems_formats(new Set<std::string>);
    gems_formats->insert("gems");

    factory->add_repository_format(
            gems_formats,
            GemsRepository::repository_factory_name,
            GemsRepository::repository_factory_create,
            GemsRepository::repository_factory_dependencies
            );

    std::tr1::shared_ptr<Set<std::string> > installed_gems_formats(new Set<std::string>);
    installed_gems_formats->insert("installed_gems");
    installed_gems_formats->insert("installed-gems");

    factory->add_repository_format(
            installed_gems_formats,
            InstalledGemsRepository::repository_factory_name,
            InstalledGemsRepository::repository_factory_create,
            InstalledGemsRepository::repository_factory_dependencies
            );
}

