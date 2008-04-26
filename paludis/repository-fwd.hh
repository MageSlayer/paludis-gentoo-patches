/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/util/set-fwd.hh>
#include <paludis/util/kc-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/keys.hh>
#include <paludis/name-fwd.hh>
#include <paludis/merger-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <tr1/memory>

/** \file
 * Forward declarations for paludis/repository.hh .
 *
 * \ingroup g_repository
 */

namespace paludis
{
    class NoSuchSetError;
    class RecursivelyDefinedSetError;

    class Environment;
    class RepositoryNameCache;
    class ERepositoryProfile;
    class Hook;
    class HookResult;

    class Repository;
    class RepositorySetsInterface;
    class RepositorySyncableInterface;
    class RepositoryUseInterface;
    class RepositoryEnvironmentVariableInterface;
    class RepositoryMirrorsInterface;
    class RepositoryProvidesInterface;
    class RepositoryVirtualsInterface;
    class RepositoryMakeVirtualsInterface;
    class RepositoryDestinationInterface;
    class RepositoryEInterface;
    class RepositoryHookInterface;
    class RepositoryQAInterface;
    class RepositoryManifestInterface;

    /**
     * A set of destinations, used to decide whether a PackageID can be
     * installed to a particular Repository.
     *
     * \ingroup g_repository
     */
    typedef Set<std::tr1::shared_ptr<Repository> > DestinationsSet;


    /**
     * Optional interfaces that may be provided by a Repository.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    typedef kc::KeyedClass<
        kc::Field<k::sets_interface, RepositorySetsInterface *>,
        kc::Field<k::syncable_interface, RepositorySyncableInterface *>,
        kc::Field<k::use_interface, RepositoryUseInterface *>,
        kc::Field<k::mirrors_interface, RepositoryMirrorsInterface *>,
        kc::Field<k::environment_variable_interface, RepositoryEnvironmentVariableInterface *>,
        kc::Field<k::provides_interface, RepositoryProvidesInterface *>,
        kc::Field<k::virtuals_interface, RepositoryVirtualsInterface *>,
        kc::Field<k::make_virtuals_interface, RepositoryMakeVirtualsInterface *>,
        kc::Field<k::destination_interface, RepositoryDestinationInterface *>,
        kc::Field<k::e_interface, RepositoryEInterface *>,
        kc::Field<k::hook_interface, RepositoryHookInterface *>,
        kc::Field<k::qa_interface, RepositoryQAInterface *>,
        kc::Field<k::manifest_interface, RepositoryManifestInterface *>
            > RepositoryCapabilities;

    /**
     * A profiles.desc line in a Repository implementing RepositoryEInterface.
     *
     * \see Repository
     * \see RepositoryEInterface
     * \ingroup g_repository
     * \nosubgrouping
     */
    typedef kc::KeyedClass<
        kc::Field<k::path, FSEntry>,
        kc::Field<k::arch, std::string>,
        kc::Field<k::status, std::string>,
        kc::Field<k::profile, std::tr1::shared_ptr<ERepositoryProfile> >
            >RepositoryEInterfaceProfilesDescLine;

    /**
     * A provides entry in a Repository implementing RepositoryProvidesInterface.
     *
     * \see Repository
     * \see RepositoryProvidesInterface
     * \ingroup g_repository
     * \nosubgrouping
     */
    typedef kc::KeyedClass<
            kc::Field<k::virtual_name, QualifiedPackageName>,
            kc::Field<k::provided_by, std::tr1::shared_ptr<const PackageID> >
        > RepositoryProvidesEntry;

    /**
     * A virtuals entry in a Repository implementing RepositoryVirtualsInterface.
     *
     * \see Repository
     * \see RepositoryVirtualsInterface
     * \ingroup g_repository
     * \nosubgrouping
     */
    typedef kc::KeyedClass<
            kc::Field<k::virtual_name, QualifiedPackageName>,
            kc::Field<k::provided_by_spec, std::tr1::shared_ptr<const PackageDepSpec> >
        > RepositoryVirtualsEntry;

    /**
     * Parameters for RepositoryDestinationInterface::merge.
     *
     * \see RepositoryDestinationInterface
     * \ingroup g_repository
     * \nosubgrouping
     */
    typedef kc::KeyedClass<
        kc::Field<k::package_id, std::tr1::shared_ptr<const PackageID> >,
        kc::Field<k::image_dir, FSEntry>,
        kc::Field<k::environment_file, FSEntry>,
        kc::Field<k::options, MergerOptions>
            > MergeParams;
}

#endif
