/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORY_FWD_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORY_FWD_HH 1

#include <paludis/util/collection-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    class Environment;
    class RepositoryNameCache;
    class PortageRepositoryProfile;
    class Hook;
    class HookResult;

    class Repository;
    class RepositoryInstallableInterface;
    class RepositoryInstalledInterface;
    class RepositoryMaskInterface;
    class RepositorySetsInterface;
    class RepositorySyncableInterface;
    class RepositoryUninstallableInterface;
    class RepositoryUseInterface;
    class RepositoryWorldInterface;
    class RepositoryEnvironmentVariableInterface;
    class RepositoryMirrorsInterface;
    class RepositoryProvidesInterface;
    class RepositoryVirtualsInterface;
    class RepositoryDestinationInterface;
    class RepositoryContentsInterface;
    class RepositoryConfigInterface;
    class RepositoryLicensesInterface;
    class RepositoryPortageInterface;
    class RepositoryHookInterface;

    /**
     * A set of destinations.
     *
     * \ingroup grpdepresolver
     */
    typedef SortedCollection<std::tr1::shared_ptr<Repository> > DestinationsCollection;

    /**
     * What debug build option to use when installing a package.
     *
     * \ingroup grprepository
     */
    enum InstallDebugOption
    {
        ido_none,
        ido_split,
        ido_internal
    };
}

#endif
