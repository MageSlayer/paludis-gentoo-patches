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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_EBUILD_METADATA_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_EBUILD_METADATA_HH 1

#include <paludis/repositories/e/e_repository_entries.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/output_manager-fwd.hh>

/** \file
 * Declaration for the EbuildEntries class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class FSEntry;
    class ERepository;

    namespace erepository
    {
        /**
         * ERepositoryEntries handler for ebuilds.
         *
         * \ingroup grperepository
         */
        class PALUDIS_VISIBLE EbuildEntries :
            public ERepositoryEntries,
            private PrivateImplementationPattern<EbuildEntries>
        {
            private:
                std::string _guess_eapi(const QualifiedPackageName &, const FSEntry &) const;

            public:
                /**
                 * Create an EbuildEntries instance.
                 */
                static std::tr1::shared_ptr<ERepositoryEntries> make_ebuild_entries(const Environment * const,
                            ERepository * const, const ERepositoryParams &);

                ///\name Basic operations
                ///\{

                EbuildEntries(const Environment * const,
                        ERepository * const e_repository,
                        const ERepositoryParams &);

                virtual ~EbuildEntries();

                ///\}

                virtual void merge(const MergeParams &);

                virtual bool is_package_file(const QualifiedPackageName &, const FSEntry &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual VersionSpec extract_package_file_version(const QualifiedPackageName &, const FSEntry &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::tr1::shared_ptr<const ERepositoryID> make_id(const QualifiedPackageName &, const FSEntry &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string get_environment_variable(const std::tr1::shared_ptr<const ERepositoryID> &, const std::string & var,
                        const std::tr1::shared_ptr<const Profile> &) const;

                virtual void fetch(const std::tr1::shared_ptr<const ERepositoryID> &,
                        const FetchAction &,
                        const std::tr1::shared_ptr<const Profile> &) const;

                virtual void pretend_fetch(const std::tr1::shared_ptr<const ERepositoryID> &, PretendFetchAction &,
                        const std::tr1::shared_ptr<const Profile> &) const;

                virtual void install(const std::tr1::shared_ptr<const ERepositoryID> &,
                        const InstallAction &,
                        const std::tr1::shared_ptr<const Profile> &) const;

                virtual bool pretend(const std::tr1::shared_ptr<const ERepositoryID> &,
                        const PretendAction &,
                        const std::tr1::shared_ptr<const Profile> &) const;

                virtual void info(const std::tr1::shared_ptr<const ERepositoryID> &,
                        const InfoAction &,
                        const std::tr1::shared_ptr<const Profile> &) const;

                virtual std::string get_package_file_manifest_key(const FSEntry &, const QualifiedPackageName &) const;

                virtual std::string binary_ebuild_name(const QualifiedPackageName &, const VersionSpec &, const std::string &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
