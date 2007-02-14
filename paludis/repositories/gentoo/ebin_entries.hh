/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_EBIN_ENTRIES_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_EBIN_ENTRIES_HH 1

#include <paludis/repositories/gentoo/portage_repository_entries.hh>
#include <paludis/repositories/gentoo/portage_repository_params.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class FSEntry;
    class PortageRepository;

    /**
     * PortageRepositoryEntries handler for ebins.
     *
     * \ingroup grpportagerepository
     */
    class PALUDIS_VISIBLE EbinEntries :
        public PortageRepositoryEntries,
        private PrivateImplementationPattern<EbinEntries>
    {
        public:
            static std::tr1::shared_ptr<PortageRepositoryEntries>
                make_ebin_entries(const Environment * const,
                        PortageRepository * const, const PortageRepositoryParams &);

            EbinEntries(const Environment * const,
                    PortageRepository * const portage_repository,
                    const PortageRepositoryParams &);

            virtual ~EbinEntries();

            virtual std::tr1::shared_ptr<VersionMetadata> generate_version_metadata(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual std::string get_environment_variable(const QualifiedPackageName &,
                    const VersionSpec &, const std::string & var,
                    std::tr1::shared_ptr<const PortageRepositoryProfile>) const;

            virtual void install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &, std::tr1::shared_ptr<const PortageRepositoryProfile>) const;

            virtual void merge(const MergeOptions &);
    };
}


#endif
