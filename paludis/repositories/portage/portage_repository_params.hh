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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_PARAMS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_PARAMS_HH 1

#include <paludis/util/smart_record.hh>
#include <paludis/util/fs_entry.hh>

/** \file
 * Declaration for the PortageRepositoryParams class.
 *
 * \ingroup grpportagerepository
 */

namespace paludis
{
    class Environment;
    class PackageDatabase;

    /**
     * Keys for PortageRepositoryParams.
     *
     * \see PortageRepositoryParams
     * \ingroup grpportagerepository
     */
    enum PortageRepositoryParamsKeys
    {
        prpk_entry_format,
        prpk_environment,
        prpk_package_database,
        prpk_location,
        prpk_profiles,
        prpk_cache,
        prpk_distdir,
        prpk_pkgdir,
        prpk_eclassdirs,
        prpk_setsdir,
        prpk_securitydir,
        prpk_newsdir,
        prpk_sync,
        prpk_sync_exclude,
        prpk_root,
        prpk_buildroot,
        last_prpk
    };

    /**
     * Tag for PortageRepositoryParams.
     *
     * \see PortageRepositoryParams
     * \ingroup grpportagerepository
     */
    struct PortageRepositoryParamsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<PortageRepositoryParamsKeys, last_prpk>,
        SmartRecordKey<prpk_entry_format, const std::string>,
        SmartRecordKey<prpk_environment, const Environment *>,
        SmartRecordKey<prpk_package_database, const PackageDatabase *>,
        SmartRecordKey<prpk_location, const FSEntry>,
        SmartRecordKey<prpk_profiles, FSEntryCollection::Pointer>,
        SmartRecordKey<prpk_cache, const FSEntry>,
        SmartRecordKey<prpk_distdir, const FSEntry>,
        SmartRecordKey<prpk_pkgdir, const FSEntry>,
        SmartRecordKey<prpk_eclassdirs, FSEntryCollection::Pointer>,
        SmartRecordKey<prpk_setsdir, const FSEntry>,
        SmartRecordKey<prpk_securitydir, const FSEntry>,
        SmartRecordKey<prpk_newsdir, const FSEntry>,
        SmartRecordKey<prpk_sync, const std::string>,
        SmartRecordKey<prpk_sync_exclude, const std::string>,
        SmartRecordKey<prpk_root, const FSEntry>,
        SmartRecordKey<prpk_buildroot, const FSEntry>
    {
    };

    /**
     * Constructor parameters for PortageRepository.
     *
     * \see PortageRepository
     * \ingroup grpportagerepository
     */
    typedef MakeSmartRecord<PortageRepositoryParamsTag>::Type PortageRepositoryParams;

}

#endif
