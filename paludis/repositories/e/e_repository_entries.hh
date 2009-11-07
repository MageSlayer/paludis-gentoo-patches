/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_METADATA_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_METADATA_HH 1

#include <paludis/name-fwd.hh>
#include <paludis/action-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/repositories/e/profile.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <tr1/memory>
#include <string>

/** \file
 * Declaration for the ERepositoryEntries class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class ERepository;

    namespace erepository
    {
        /**
         * Handle entries (for example, ebuilds) in a ERepository.
         *
         * \ingroup grperepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE ERepositoryEntries
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~ERepositoryEntries() = 0;

                ///\}

                virtual bool is_package_file(const QualifiedPackageName &, const FSEntry &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual VersionSpec extract_package_file_version(const QualifiedPackageName &, const FSEntry &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                /**
                 * Create an ERepositoryID.
                 */
                virtual const std::tr1::shared_ptr<const ERepositoryID> make_id(const QualifiedPackageName &, const FSEntry &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                /**
                 * Fetch an environment variable.
                 */
                virtual std::string get_environment_variable(const std::tr1::shared_ptr<const ERepositoryID> &, const std::string & var,
                        const std::tr1::shared_ptr<const erepository::Profile> &) const = 0;

                /**
                 * Handle an install.
                 */
                virtual void install(const std::tr1::shared_ptr<const ERepositoryID> &, const InstallAction &,
                        const std::tr1::shared_ptr<const erepository::Profile> &) const = 0;

                /**
                 * Handle a fetch.
                 */
                virtual void fetch(const std::tr1::shared_ptr<const ERepositoryID> &,
                        const FetchAction &,
                        const std::tr1::shared_ptr<const erepository::Profile> &) const = 0;

                /**
                 * Handle a pretend fetch.
                 */
                virtual void pretend_fetch(const std::tr1::shared_ptr<const ERepositoryID> &, PretendFetchAction &,
                        const std::tr1::shared_ptr<const erepository::Profile> &) const = 0;

                /**
                 * Handle a pretend.
                 */
                virtual bool pretend(const std::tr1::shared_ptr<const ERepositoryID> &,
                        const PretendAction &,
                        const std::tr1::shared_ptr<const erepository::Profile> &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                /**
                 * Handle an info.
                 */
                virtual void info(const std::tr1::shared_ptr<const ERepositoryID> &,
                        const InfoAction &,
                        const std::tr1::shared_ptr<const erepository::Profile> &) const = 0;

                /**
                 * Handle a merge.
                 */
                virtual void merge(const MergeParams &) = 0;

                /**
                 * Gives the Manifest key for a given package file (for
                 * example, "EBUILD").
                 */
                virtual std::string get_package_file_manifest_key(const FSEntry &, const QualifiedPackageName &) const = 0;

                /**
                 * Generate the name for a binary ebuild.
                 */
                virtual std::string binary_ebuild_name(const QualifiedPackageName &, const VersionSpec &, const std::string &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
        };

        /**
         * Virtual constructor for ERepositoryEntries.
         *
         * \ingroup grprepository
         */
        class PALUDIS_VISIBLE ERepositoryEntriesFactory :
            public InstantiationPolicy<ERepositoryEntriesFactory, instantiation_method::SingletonTag>
        {
            friend class InstantiationPolicy<ERepositoryEntriesFactory, instantiation_method::SingletonTag>;

            private:
                ERepositoryEntriesFactory();

            public:
                const std::tr1::shared_ptr<ERepositoryEntries> create(
                        const std::string &,
                        const Environment * const,
                        ERepository * const,
                        const ERepositoryParams &)
                    const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
