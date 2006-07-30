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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_EBUILD_METADATA_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_EBUILD_METADATA_HH 1

#include <paludis/repositories/portage/portage_repository_entries.hh>
#include <paludis/repositories/portage/portage_repository_params.hh>
#include <paludis/util/private_implementation_pattern.hh>

/** \file
 * Declaration for the PortageRepositoryEbuildEntries class.
 *
 * \ingroup grpportagerepository
 */

namespace paludis
{
    class FSEntry;
    class PortageRepository;

    /**
     * PortageRepositoryEntries handler for ebuilds.
     *
     * \ingroup grpportagerepository
     */
    class PortageRepositoryEbuildEntries :
        public PortageRepositoryEntries,
        private PrivateImplementationPattern<PortageRepositoryEbuildEntries>
    {
        public:
            static PortageRepositoryEbuildEntries::Pointer
                make_portage_repository_ebuild_entries(const Environment * const,
                        PortageRepository * const, const PortageRepositoryParams &);

            PortageRepositoryEbuildEntries(const Environment * const,
                    PortageRepository * const portage_repository,
                    const PortageRepositoryParams &);

            virtual ~PortageRepositoryEbuildEntries();

            virtual VersionMetadata::Pointer generate_version_metadata(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual std::string get_environment_variable(const QualifiedPackageName &,
                    const VersionSpec &, const std::string & var,
                    PortageRepositoryProfile::ConstPointer) const;

            virtual void install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &, PortageRepositoryProfile::ConstPointer) const;
    };

    /**
     * Register PortageRepositoryEbuildEntries.
     *
     * \ingroup grpportagerepository
     */
    static const PortageRepositoryEntriesMaker::RegisterMaker register_portage_repository_ebuild_entries(
            "ebuild", &PortageRepositoryEbuildEntries::make_portage_repository_ebuild_entries);


}

#endif
