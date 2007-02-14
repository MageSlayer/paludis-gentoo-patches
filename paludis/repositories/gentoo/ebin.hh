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

#ifndef PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_GENTOO_EBIN_HH
#define PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_GENTOO_EBIN_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/collection.hh>
#include <paludis/package_database.hh>
#include <string>

namespace paludis
{
    class Command;

#include <paludis/repositories/gentoo/ebin-se.hh>
#include <paludis/repositories/gentoo/ebin-sr.hh>

    class EbinVersionMetadata :
        public VersionMetadata,
        public VersionMetadataEbuildInterface,
        public VersionMetadataEbinInterface,
        public VersionMetadataDepsInterface,
        public VersionMetadataLicenseInterface,
        public VersionMetadataOriginsInterface
    {
        public:
            EbinVersionMetadata(const SlotName &);
            virtual ~EbinVersionMetadata();
    };

    class EbinCommand :
        private InstantiationPolicy<EbinCommand, instantiation_method::NonCopyableTag>
    {
        protected:
            EbinCommandParams params;

            EbinCommand(const EbinCommandParams &);

            virtual std::string commands() const = 0;
            virtual bool success();
            virtual bool use_sandbox() const;
            virtual bool failure() = 0;
            virtual bool do_run_command(const Command &);
            virtual Command extend_command(const Command &) = 0;

        public:
            virtual ~EbinCommand();

            virtual bool operator() ();
    };

    class EbinFetchCommand :
        public EbinCommand
    {
        protected:
            const EbinFetchCommandParams fetch_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual Command extend_command(const Command &);

        public:
            EbinFetchCommand(const EbinCommandParams &, const EbinFetchCommandParams &);
    };

    class EbinInstallCommand :
        public EbinCommand
    {
        protected:
            const EbinInstallCommandParams install_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual Command extend_command(const Command &);

        public:
            EbinInstallCommand(const EbinCommandParams &, const EbinInstallCommandParams &);
    };

    class EbinMergeCommand
    {
        protected:
            const EbinCommandParams params;
            const EbinMergeCommandParams merge_params;

        public:
            EbinMergeCommand(const EbinCommandParams &, const EbinMergeCommandParams &);

            void operator() ();
    };
}

#endif

